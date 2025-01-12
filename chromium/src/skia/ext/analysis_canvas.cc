// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "base/trace_event/trace_event.h"
#include "skia/ext/analysis_canvas.h"
#include "third_party/skia/include/core/SkDraw.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkRRect.h"
#include "third_party/skia/include/core/SkShader.h"

namespace {

const int kNoLayer = -1;

const int kMaxSmallLosslessBitmapWidth = 32;
const int kMaxSmallLosslessBitmapHeight = 32;

bool ActsLikeClear(SkXfermode::Mode mode, unsigned src_alpha) {
  switch (mode) {
    case SkXfermode::kClear_Mode:
      return true;
    case SkXfermode::kSrc_Mode:
    case SkXfermode::kSrcIn_Mode:
    case SkXfermode::kDstIn_Mode:
    case SkXfermode::kSrcOut_Mode:
    case SkXfermode::kDstATop_Mode:
      return src_alpha == 0;
    case SkXfermode::kDstOut_Mode:
      return src_alpha == 0xFF;
    default:
      return false;
  }
}

bool IsSolidColorPaint(const SkPaint& paint) {
  SkXfermode::Mode xfermode;

  // getXfermode can return a NULL, but that is handled
  // gracefully by AsMode (NULL turns into kSrcOver mode).
  if (!SkXfermode::AsMode(paint.getXfermode(), &xfermode))
    return false;

  // Paint is solid color if the following holds:
  // - Alpha is 1.0, style is fill, and there are no special effects
  // - Xfer mode is either kSrc or kSrcOver (kSrcOver is equivalent
  //   to kSrc if source alpha is 1.0, which is already checked).
  return (paint.getAlpha() == 255 &&
          !paint.getShader() &&
          !paint.getLooper() &&
          !paint.getMaskFilter() &&
          !paint.getColorFilter() &&
          !paint.getImageFilter() &&
          paint.getStyle() == SkPaint::kFill_Style &&
          (xfermode == SkXfermode::kSrc_Mode ||
           xfermode == SkXfermode::kSrcOver_Mode));
}

bool IsGradientPaint(const SkPaint& paint) {
  if (IsSolidColorPaint(paint))
    return false;

  SkShader* shader = paint.getShader();
  return shader && shader->asAGradient(NULL) != SkShader::kNone_GradientType;
}

// Returns true if the specified drawn_rect will cover the entire canvas, and
// that the canvas is not clipped (i.e. it covers ALL of the canvas).
bool IsFullQuad(SkCanvas* canvas, const SkRect& drawn_rect) {
  if (!canvas->isClipRect())
    return false;

  SkIRect clip_irect;
  if (!canvas->getClipDeviceBounds(&clip_irect))
    return false;

  // if the clip is smaller than the canvas, we're partly clipped, so abort.
  if (!clip_irect.contains(SkIRect::MakeSize(canvas->getBaseLayerSize())))
    return false;

  const SkMatrix& matrix = canvas->getTotalMatrix();
  // If the transform results in a non-axis aligned
  // rect, then be conservative and return false.
  if (!matrix.rectStaysRect())
    return false;

  SkRect device_rect;
  matrix.mapRect(&device_rect, drawn_rect);
  SkRect clip_rect;
  clip_rect.set(clip_irect);
  return device_rect.contains(clip_rect);
}

} // namespace

namespace skia {

void AnalysisCanvas::SetForceNotSolid(bool flag) {
  is_forced_not_solid_ = flag;
  if (is_forced_not_solid_)
    is_solid_color_ = false;
}

void AnalysisCanvas::SetForceNotTransparent(bool flag) {
  is_forced_not_transparent_ = flag;
  if (is_forced_not_transparent_)
    is_transparent_ = false;
}

void AnalysisCanvas::onDrawPaint(const SkPaint& paint) {
  TRACE_EVENT0("disabled-by-default-skia", "AnalysisCanvas::onDrawPaint");
  SkRect rect;
  if (getClipBounds(&rect))
    drawRect(rect, paint);
}

void AnalysisCanvas::onDrawPoints(SkCanvas::PointMode mode,
                                  size_t count,
                                  const SkPoint points[],
                                  const SkPaint& paint) {
  TRACE_EVENT0("disabled-by-default-skia", "AnalysisCanvas::onDrawPoints");
  is_solid_color_ = false;
  is_transparent_ = false;
  ++draw_op_count_;
}

void AnalysisCanvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
  TRACE_EVENT0("disabled-by-default-skia", "AnalysisCanvas::onDrawRect");
  // This recreates the early-exit logic in SkCanvas.cpp.
  SkRect scratch;
  if (paint.canComputeFastBounds() &&
      quickReject(paint.computeFastBounds(rect, &scratch))) {
    TRACE_EVENT_INSTANT0("disabled-by-default-skia", "Quick reject.",
                         TRACE_EVENT_SCOPE_THREAD);
    return;
  }

  // An extra no-op check SkCanvas.cpp doesn't do.
  if (paint.nothingToDraw()) {
    TRACE_EVENT_INSTANT0("disabled-by-default-skia", "Nothing to draw.",
                         TRACE_EVENT_SCOPE_THREAD);
    return;
  }

  bool does_cover_canvas = IsFullQuad(this, rect);

  SkXfermode::Mode xfermode;
  SkXfermode::AsMode(paint.getXfermode(), &xfermode);

  // This canvas will become transparent if the following holds:
  // - The quad is a full tile quad
  // - We're not in "forced not transparent" mode
  // - Transfer mode is clear (0 color, 0 alpha)
  //
  // If the paint alpha is not 0, or if the transfrer mode is
  // not src, then this canvas will not be transparent.
  //
  // In all other cases, we keep the current transparent value
  if (does_cover_canvas &&
      !is_forced_not_transparent_ &&
      ActsLikeClear(xfermode, paint.getAlpha())) {
    is_transparent_ = true;
    has_large_lossless_bitmap_ = false;
    has_gradient_ = false;
  } else if (paint.getAlpha() != 0 || xfermode != SkXfermode::kSrc_Mode) {
    is_transparent_ = false;
  }

  // This bitmap is solid if and only if the following holds.
  // Note that this might be overly conservative:
  // - We're not in "forced not solid" mode
  // - Paint is solid color
  // - The quad is a full tile quad
  if (!is_forced_not_solid_ && IsSolidColorPaint(paint) && does_cover_canvas) {
    is_solid_color_ = true;
    color_ = paint.getColor();
    has_large_lossless_bitmap_ = false;
    has_gradient_ = false;
  } else {
    is_solid_color_ = false;

    if (IsGradientPaint(paint)) {
      has_gradient_ = true;
    }
  }
  ++draw_op_count_;
}

void AnalysisCanvas::onDrawOval(const SkRect& oval, const SkPaint& paint) {
  TRACE_EVENT0("disabled-by-default-skia", "AnalysisCanvas::onDrawOval");
  SkRect storage;
  const SkRect* bounds = NULL;
  if (paint.canComputeFastBounds()) {
    bounds = &paint.computeFastBounds(oval, &storage);
    if (this->quickReject(*bounds)) {
      return;
    }
  }

  is_solid_color_ = false;
  is_transparent_ = false;

  if (IsGradientPaint(paint)) {
    has_gradient_ = true;
  }

  ++draw_op_count_;
}

void AnalysisCanvas::onDrawArc(const SkRect& oval,
                               SkScalar startAngle,
                               SkScalar sweepAngle,
                               bool useCenter,
                               const SkPaint& paint) {
  TRACE_EVENT0("disabled-by-default-skia", "AnalysisCanvas::onDrawArc");
  is_solid_color_ = false;
  is_transparent_ = false;
  ++draw_op_count_;
}

void AnalysisCanvas::onDrawRRect(const SkRRect& rr, const SkPaint& paint) {
  TRACE_EVENT0("disabled-by-default-skia", "AnalysisCanvas::onDrawRRect");
  SkRect storage;
  const SkRect* bounds = NULL;
  if (paint.canComputeFastBounds()) {
    bounds = &paint.computeFastBounds(rr.getBounds(), &storage);
    if (this->quickReject(*bounds)) {
      return;
    }
  }

  // This should add the SkRRect to an SkPath, and call
  // drawPath, but since drawPath ignores the SkPath, just
  // do the same work here.
  is_solid_color_ = false;
  is_transparent_ = false;

  if (IsGradientPaint(paint)) {
    has_gradient_ = true;
  }

  ++draw_op_count_;
}

void AnalysisCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
  TRACE_EVENT0("disabled-by-default-skia", "AnalysisCanvas::onDrawPath");
  is_solid_color_ = false;
  is_transparent_ = false;
  ++draw_op_count_;
}

void AnalysisCanvas::onDrawBitmap(const SkBitmap& bitmap,
                                  SkScalar left,
                                  SkScalar top,
                                  const SkPaint*) {
  TRACE_EVENT0("disabled-by-default-skia", "AnalysisCanvas::onDrawBitmap");
  is_solid_color_ = false;
  is_transparent_ = false;
  ++draw_op_count_;
}

void AnalysisCanvas::onDrawBitmapRect(const SkBitmap&,
                                      const SkRect* src,
                                      const SkRect& dst,
                                      const SkPaint* paint,
                                      SrcRectConstraint) {
  TRACE_EVENT0("disabled-by-default-skia", "AnalysisCanvas::onDrawBitmapRect");
  // Call drawRect to determine transparency,
  // but reset solid color to false.
  SkPaint tmpPaint;
  if (!paint)
    paint = &tmpPaint;
  drawRect(dst, *paint);
  is_solid_color_ = false;
  ++draw_op_count_;
}

void AnalysisCanvas::onDrawBitmapNine(const SkBitmap& bitmap,
                                      const SkIRect& center,
                                      const SkRect& dst,
                                      const SkPaint* paint) {
  TRACE_EVENT0("disabled-by-default-skia", "AnalysisCanvas::onDrawBitmapNine");
  is_solid_color_ = false;
  is_transparent_ = false;
  ++draw_op_count_;
}

void AnalysisCanvas::onDrawImage(const SkImage* image,
                                 SkScalar left,
                                 SkScalar top,
                                 const SkPaint* paint) {
  TRACE_EVENT0("disabled-by-default-skia", "AnalysisCanvas::onDrawImage");
  SkRect bounds = SkRect::MakeXYWH(left, top, SkIntToScalar(image->width()),
                                   SkIntToScalar(image->height()));
  if (NULL == paint || paint->canComputeFastBounds()) {
    if (paint) {
      paint->computeFastBounds(bounds, &bounds);
    }
    if (this->quickReject(bounds)) {
      return;
    }
  }

  is_solid_color_ = false;
  is_transparent_ = false;
  has_large_lossless_bitmap_ |=
      (image->width() > kMaxSmallLosslessBitmapWidth ||
       image->height() > kMaxSmallLosslessBitmapHeight) &&
      image->isDecodedWithLosslessCodec();
  ++draw_op_count_;
}

void AnalysisCanvas::onDrawImageRect(const SkImage* image,
                                     const SkRect* src,
                                     const SkRect& dst,
                                     const SkPaint* paint,
                                     SrcRectConstraint) {
  TRACE_EVENT0("disabled-by-default-skia", "AnalysisCanvas::onDrawImageRect");
  SkRect storage;
  const SkRect* bounds = &dst;
  if (NULL == paint || paint->canComputeFastBounds()) {
    if (paint) {
      bounds = &paint->computeFastBounds(dst, &storage);
    }
    if (this->quickReject(*bounds)) {
      return;
    }
  }

  // Call drawRect to determine transparency,
  // but reset solid color to false.
  SkPaint tmpPaint;
  if (!paint)
    paint = &tmpPaint;
  drawRect(dst, *paint);
  is_solid_color_ = false;
  has_large_lossless_bitmap_ |=
      (dst.width() > kMaxSmallLosslessBitmapWidth ||
       dst.height() > kMaxSmallLosslessBitmapHeight) &&
      image->isDecodedWithLosslessCodec();
  ++draw_op_count_;
}

void AnalysisCanvas::onDrawText(const void* text,
                                size_t len,
                                SkScalar x,
                                SkScalar y,
                                const SkPaint& paint) {
  TRACE_EVENT0("disabled-by-default-skia", "AnalysisCanvas::onDrawText");
  is_solid_color_ = false;
  is_transparent_ = false;
  ++draw_op_count_;
}

void AnalysisCanvas::onDrawPosText(const void* text,
                                   size_t byteLength,
                                   const SkPoint pos[],
                                   const SkPaint& paint) {
  TRACE_EVENT0("disabled-by-default-skia", "AnalysisCanvas::onDrawPosText");
  is_solid_color_ = false;
  is_transparent_ = false;
  ++draw_op_count_;
}

void AnalysisCanvas::onDrawPosTextH(const void* text,
                                    size_t byteLength,
                                    const SkScalar xpos[],
                                    SkScalar constY,
                                    const SkPaint& paint) {
  TRACE_EVENT0("disabled-by-default-skia", "AnalysisCanvas::onDrawPosTextH");
  is_solid_color_ = false;
  is_transparent_ = false;
  ++draw_op_count_;
}

void AnalysisCanvas::onDrawTextOnPath(const void* text,
                                      size_t len,
                                      const SkPath& path,
                                      const SkMatrix* matrix,
                                      const SkPaint& paint) {
  TRACE_EVENT0("disabled-by-default-skia", "AnalysisCanvas::onDrawTextOnPath");
  is_solid_color_ = false;
  is_transparent_ = false;
  ++draw_op_count_;
}

void AnalysisCanvas::onDrawTextBlob(const SkTextBlob* blob,
                                    SkScalar x,
                                    SkScalar y,
                                    const SkPaint &paint) {
  TRACE_EVENT0("disabled-by-default-skia", "AnalysisCanvas::onDrawTextBlob");
  is_solid_color_ = false;
  is_transparent_ = false;
  ++draw_op_count_;
}

void AnalysisCanvas::onDrawDRRect(const SkRRect& outer,
                                  const SkRRect& inner,
                                  const SkPaint& paint) {
  TRACE_EVENT0("disabled-by-default-skia", "AnalysisCanvas::onDrawDRRect");
  is_solid_color_ = false;
  is_transparent_ = false;
  ++draw_op_count_;
}

void AnalysisCanvas::onDrawVertices(SkCanvas::VertexMode,
                                    int vertex_count,
                                    const SkPoint verts[],
                                    const SkPoint texs[],
                                    const SkColor colors[],
                                    SkXfermode* xmode,
                                    const uint16_t indices[],
                                    int index_count,
                                    const SkPaint& paint) {
  TRACE_EVENT0("disabled-by-default-skia", "AnalysisCanvas::onDrawVertices");
  is_solid_color_ = false;
  is_transparent_ = false;
  ++draw_op_count_;
}

// Needed for now, since SkCanvas requires a bitmap, even if it is not backed
// by any pixels
static SkBitmap MakeEmptyBitmap(int width, int height) {
  SkBitmap bitmap;
  bitmap.setInfo(SkImageInfo::MakeUnknown(width, height));
  return bitmap;
}

AnalysisCanvas::AnalysisCanvas(int width, int height, int analyze_flags)
    : INHERITED(MakeEmptyBitmap(width, height)),
      analyze_flags_(analyze_flags),
      saved_stack_size_(0),
      force_not_solid_stack_level_(kNoLayer),
      force_not_transparent_stack_level_(kNoLayer),
      is_forced_not_solid_(false),
      is_forced_not_transparent_(false),
      is_solid_color_(true),
      color_(SK_ColorTRANSPARENT),
      is_transparent_(true),
      has_large_lossless_bitmap_(false),
      has_gradient_(false),
      draw_op_count_(0) {}

AnalysisCanvas::~AnalysisCanvas() {}

bool AnalysisCanvas::GetColorIfSolid(SkColor* color) const {
  if (is_transparent_) {
    *color = SK_ColorTRANSPARENT;
    return true;
  }
  if (is_solid_color_) {
    *color = color_;
    return true;
  }
  return false;
}

bool AnalysisCanvas::HasLargeLosslessBitmap() const {
  return has_large_lossless_bitmap_;
}

bool AnalysisCanvas::HasGradient() const {
  return has_gradient_;
}

bool AnalysisCanvas::abort() {
  if (!(analyze_flags_ & ANALYZE_LOSSLESS_BITMAP) &&
      !(analyze_flags_ & ANALYZE_GRADIENT)) {
    // Early out as soon as we have more than one draw op.
    // TODO(vmpstr): Investigate if 1 is the correct metric here. We need to
    // balance the amount of time we spend analyzing vs how many tiles would be
    // solid if the number was higher.
    if (draw_op_count_ > 1) {
      TRACE_EVENT0("disabled-by-default-skia",
                   "AnalysisCanvas::abort() -- aborting");
      // We have to reset solid/transparent state to false since we don't
      // know whether consequent operations will make this false.
      is_solid_color_ = false;
      is_transparent_ = false;
      return true;
    }
  } else if (((analyze_flags_ & ANALYZE_LOSSLESS_BITMAP) &&
              HasLargeLosslessBitmap()) ||
             ((analyze_flags_ & ANALYZE_GRADIENT) && HasGradient())) {
    return true;
  }
  return false;
}

void AnalysisCanvas::OnComplexClip() {
  // complex clips can make our calls to IsFullQuad invalid (ie have false
  // positives). As a precaution, force the setting to be non-solid
  // and non-transparent until we pop this
  if (force_not_solid_stack_level_ == kNoLayer) {
    force_not_solid_stack_level_ = saved_stack_size_;
    SetForceNotSolid(true);
  }
  if (force_not_transparent_stack_level_ == kNoLayer) {
    force_not_transparent_stack_level_ = saved_stack_size_;
    SetForceNotTransparent(true);
  }
}

void AnalysisCanvas::onClipRect(const SkRect& rect,
                                SkRegion::Op op,
                                ClipEdgeStyle edge_style) {
  INHERITED::onClipRect(rect, op, edge_style);
}

void AnalysisCanvas::onClipPath(const SkPath& path,
                                SkRegion::Op op,
                                ClipEdgeStyle edge_style) {
  OnComplexClip();
  INHERITED::onClipRect(path.getBounds(), op, edge_style);
}

void AnalysisCanvas::onClipRRect(const SkRRect& rrect,
                                 SkRegion::Op op,
                                 ClipEdgeStyle edge_style) {
  OnComplexClip();
  INHERITED::onClipRect(rrect.getBounds(), op, edge_style);
}

void AnalysisCanvas::onClipRegion(const SkRegion& deviceRgn, SkRegion::Op op) {
  const ClipEdgeStyle edge_style = kHard_ClipEdgeStyle;
  if (deviceRgn.isRect()) {
    onClipRect(SkRect::MakeFromIRect(deviceRgn.getBounds()), op, edge_style);
    return;
  }
  OnComplexClip();
  INHERITED::onClipRect(
      SkRect::MakeFromIRect(deviceRgn.getBounds()), op, edge_style);
}

void AnalysisCanvas::willSave() {
  ++saved_stack_size_;
  INHERITED::willSave();
}

SkCanvas::SaveLayerStrategy AnalysisCanvas::getSaveLayerStrategy(
    const SaveLayerRec& rec) {
  const SkPaint* paint = rec.fPaint;

  ++saved_stack_size_;

  SkIRect canvas_ibounds = SkIRect::MakeSize(this->getBaseLayerSize());
  SkRect canvas_bounds;
  canvas_bounds.set(canvas_ibounds);

  // If after we draw to the saved layer, we have to blend with the current
  // layer, then we can conservatively say that the canvas will not be of
  // solid color.
  if ((paint && !IsSolidColorPaint(*paint)) ||
      (rec.fBounds && !rec.fBounds->contains(canvas_bounds))) {
    if (force_not_solid_stack_level_ == kNoLayer) {
      force_not_solid_stack_level_ = saved_stack_size_;
      SetForceNotSolid(true);
    }
  }

  // If after we draw to the save layer, we have to blend with the current
  // layer using any part of the current layer's alpha, then we can
  // conservatively say that the canvas will not be transparent.
  SkXfermode::Mode xfermode = SkXfermode::kSrc_Mode;
  if (paint)
    SkXfermode::AsMode(paint->getXfermode(), &xfermode);
  if (xfermode != SkXfermode::kDst_Mode) {
    if (force_not_transparent_stack_level_ == kNoLayer) {
      force_not_transparent_stack_level_ = saved_stack_size_;
      SetForceNotTransparent(true);
    }
  }

  INHERITED::getSaveLayerStrategy(rec);
  // Actually saving a layer here could cause a new bitmap to be created
  // and real rendering to occur.
  return kNoLayer_SaveLayerStrategy;
}

void AnalysisCanvas::willRestore() {
  DCHECK(saved_stack_size_);
  if (saved_stack_size_) {
    --saved_stack_size_;
    if (saved_stack_size_ < force_not_solid_stack_level_) {
      SetForceNotSolid(false);
      force_not_solid_stack_level_ = kNoLayer;
    }
    if (saved_stack_size_ < force_not_transparent_stack_level_) {
      SetForceNotTransparent(false);
      force_not_transparent_stack_level_ = kNoLayer;
    }
  }

  INHERITED::willRestore();
}

}  // namespace skia


