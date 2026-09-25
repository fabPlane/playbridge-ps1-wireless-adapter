import AppKit
import Foundation

guard CommandLine.arguments.count == 4 else { fatalError("usage: annotate input output mode") }
let input = CommandLine.arguments[1], output = CommandLine.arguments[2], mode = CommandLine.arguments[3]
guard let image = NSImage(contentsOfFile: input) else { fatalError("cannot load input") }
let size = image.size
let canvas = NSImage(size: size)

func label(_ text: String, x: CGFloat, y: CGFloat, font: CGFloat = 24) {
    let attrs: [NSAttributedString.Key: Any] = [
        .font: NSFont.systemFont(ofSize: font, weight: .bold),
        .foregroundColor: NSColor.white,
        .strokeColor: NSColor.black,
        .strokeWidth: -3.0
    ]
    let s = NSAttributedString(string: text, attributes: attrs)
    let bounds = s.size()
    let box = NSRect(x: x - 8, y: y - 5, width: bounds.width + 16, height: bounds.height + 10)
    NSColor(calibratedWhite: 0.05, alpha: 0.72).setFill()
    NSBezierPath(roundedRect: box, xRadius: 7, yRadius: 7).fill()
    s.draw(at: NSPoint(x: x, y: y))
}

canvas.lockFocus()
image.draw(in: NSRect(origin: .zero, size: size))
switch mode {
case "top":
    label("J1", x: 470, y: 58)
    label("J2", x: 790, y: 58)
    label("J3", x: 1260, y: 790)
    label("J4", x: 1390, y: 625)
    label("1 GND", x: 1440, y: 590, font: 18)
    label("2 3V3", x: 1440, y: 556, font: 18)
    label("3 TX", x: 1440, y: 522, font: 18)
    label("4 RX", x: 1440, y: 488, font: 18)
case "perspective":
    label("J1", x: 485, y: 70)
    label("J2", x: 800, y: 55)
    label("J3", x: 1280, y: 790)
    label("J4", x: 1390, y: 570)
    label("GND · 3V3 · TX · RX", x: 1320, y: 520, font: 18)
case "j4":
    label("J4 — 3.3 V UART ONLY", x: 1450, y: 760, font: 25)
    label("1  GND", x: 1640, y: 665, font: 22)
    label("2  3V3", x: 1640, y: 605, font: 22)
    label("3  TX", x: 1640, y: 545, font: 22)
    label("4  RX", x: 1640, y: 485, font: 22)
default: fatalError("unknown mode")
}
canvas.unlockFocus()
guard let tiff = canvas.tiffRepresentation,
      let rep = NSBitmapImageRep(data: tiff),
      let png = rep.representation(using: .png, properties: [:]) else { fatalError("encode") }
try png.write(to: URL(fileURLWithPath: output))
