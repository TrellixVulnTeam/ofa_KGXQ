/*
 Copyright 2016-present Google Inc. All Rights Reserved.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

import MDFTextAccessibility
import UIKit

private let cellReuseIdentifier = "ColorCell"
private let headerReuseIdentifier = "ColorHeader"

/** Return a hex string like "#3FA066" from a UIColor object. */
private func hexStringFromColor(color: UIColor) -> String {
  var red: CGFloat = 0
  var green: CGFloat = 0
  var blue: CGFloat = 0
  var alpha: CGFloat = 0
  color.getRed(&red, green: &green, blue: &blue, alpha: &alpha)
  return String(format:"#%.2X%.2X%.2X", Int(red * 255), Int(green * 255), Int(blue * 255))
}

/** Return a uniformly random opaque RGB color. */
private func randomRGBColor() -> UIColor {
  return UIColor(red: CGFloat(arc4random_uniform(256)) / CGFloat(255),
                 green: CGFloat(arc4random_uniform(256)) / CGFloat(255),
                 blue: CGFloat(arc4random_uniform(256)) / CGFloat(255),
                 alpha: 1)
}

/** Set the textColor of a UILabel based on a target text alpha and a background color. */
private func setLabelAccessibleTextColor(label: UILabel,
                                         targetTextAlpha: CGFloat,
                                         preferLightText: Bool,
                                         onBackgroundColor backgroundColor: UIColor?) {
  if (backgroundColor != nil) {
    let options : MDFTextAccessibilityOptions = [
      MDFTextAccessibility.isLargeForContrastRatios(label.font) ? .LargeFont : .None,
      preferLightText ? .PreferLighter : .PreferDarker
    ]

    label.textColor =
      MDFTextAccessibility.textColorOnBackgroundColor(backgroundColor!,
                                                      targetTextAlpha: targetTextAlpha,
                                                      options: options)
  }
}

/** A title containing a contrast ratio for a color ship. */
private func contrastRatioTitle(prefix: String,
                                 textColor: UIColor,
                                 backgroundColor: UIColor) -> String {
  let ratio = MDFTextAccessibility.contrastRatioForTextColor(textColor,
                                                             onBackgroundColor: backgroundColor);
  return prefix + String(format: " %.1f:1", ratio)
}

/** A title reporting the background color for a color chip. */
private func backgroundColorTitle(prefix: String, backgroundColor: UIColor) -> String {
  return hexStringFromColor(backgroundColor)
}

/** A section of color chips with a title text accessibility options. */
class ColorChipSection {
  var title = ""
  var colors = [UIColor]()
  var targetTextAlpha: CGFloat = 1
  var prefersLightText = true
}

/** A collection view of color chips. */
class ColorsCollectionViewController: UICollectionViewController {
  var colorChipSections = [ColorChipSection]()

  override init(collectionViewLayout layout: UICollectionViewLayout) {
    super.init(collectionViewLayout: layout)
    self.createColorChipSections()
  }

  required init?(coder aDecoder: NSCoder) {
    super.init(coder:aDecoder)
    self.createColorChipSections()
  }

  override func viewDidLoad() {
    let flowLayout = self.collectionViewLayout as! UICollectionViewFlowLayout
    flowLayout.sectionInset = UIEdgeInsetsMake(0, 8, 8, 8)
  }

  private func createColorChipSections() {
    let numColorsPerSection = 10
    var colors = [UIColor]()
    for _ in 1...numColorsPerSection {
      colors.append(randomRGBColor())
    }

    // Fairly typical usage: non-opaque text with a preference to light text.
    var colorChipSection = ColorChipSection()
    colorChipSection.title = "Target alpha of 0.87, prefer light text"
    colorChipSection.targetTextAlpha = 0.87
    colorChipSection.colors = colors
    self.colorChipSections.append(colorChipSection)

    // Non-opaque text with a preference to dark text.
    colorChipSection = ColorChipSection()
    colorChipSection.title = "Target alpha of 0.87, prefer dark text"
    colorChipSection.targetTextAlpha = 0.87
    colorChipSection.prefersLightText = false
    colorChipSection.colors = colors
    self.colorChipSections.append(colorChipSection)

    // Minimal opacity text.
    colorChipSection = ColorChipSection()
    colorChipSection.title = "Minimally-opaque accessible text"
    colorChipSection.targetTextAlpha = 0
    colorChipSection.colors = colors
    self.colorChipSections.append(colorChipSection)
  }

  private func backgroundColorForIndexPath(indexPath: NSIndexPath) -> UIColor {
    assert(indexPath.section < self.colorChipSections.count)
    assert(indexPath.row < self.colorChipSections[indexPath.section].colors.count)
    return self.colorChipSections[indexPath.section].colors[indexPath.row]
  }

  private func configureCell(cell: ColorCollectionViewCell,
                             forItemAtIndexPath indexPath: NSIndexPath) {
    let colorChipSection = self.colorChipSections[indexPath.section]
    cell.backgroundColor = colorChipSection.colors[indexPath.row]

    let labels = [ cell.backgroundColorLabel, cell.largeTextLabel, cell.normalTextLabel ]
    for label in labels {
      setLabelAccessibleTextColor(label,
                                  targetTextAlpha: colorChipSection.targetTextAlpha,
                                  preferLightText: colorChipSection.prefersLightText,
                                  onBackgroundColor: cell.backgroundColor)
    }

    cell.backgroundColorLabel.text = backgroundColorTitle("Background ",
                                                          backgroundColor:cell.backgroundColor!)
    cell.largeTextLabel.text = contrastRatioTitle("Large text",
                                                   textColor:cell.largeTextLabel.textColor,
                                                   backgroundColor:cell.backgroundColor!)
    cell.normalTextLabel.text = contrastRatioTitle("Normal text",
                                                    textColor:cell.normalTextLabel.textColor,
                                                    backgroundColor:cell.backgroundColor!)
  }
}

// UICollectionViewDataSource methods
extension ColorsCollectionViewController {
  override func numberOfSectionsInCollectionView(collectionView: UICollectionView) -> Int {
    return self.colorChipSections.count
  }

  override func collectionView(collectionView: UICollectionView,
                               numberOfItemsInSection section: Int) -> Int {
    return self.colorChipSections[section].colors.count
  }

  override func collectionView(
    collectionView: UICollectionView,
    cellForItemAtIndexPath indexPath: NSIndexPath) -> UICollectionViewCell {
    let cell = collectionView.dequeueReusableCellWithReuseIdentifier(cellReuseIdentifier,
                                                                     forIndexPath: indexPath)
    self.configureCell(cell as! ColorCollectionViewCell, forItemAtIndexPath: indexPath)
    return cell
  }

  override func collectionView(collectionView: UICollectionView,
                               viewForSupplementaryElementOfKind kind: String,
                               atIndexPath indexPath: NSIndexPath) -> UICollectionReusableView {
    assert(kind == UICollectionElementKindSectionHeader)
    let headerView =
        collectionView.dequeueReusableSupplementaryViewOfKind(kind,
                                                              withReuseIdentifier: headerReuseIdentifier,
                                                              forIndexPath: indexPath)
        as! ColorCollectionViewHeader
    headerView.titleLabel.text = self.colorChipSections[indexPath.section].title
    return headerView
    }
  }