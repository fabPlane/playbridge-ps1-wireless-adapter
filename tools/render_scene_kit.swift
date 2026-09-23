import Foundation
import AppKit
import SceneKit

guard CommandLine.arguments.count >= 3 else { fatalError("usage: render_scene_kit.swift input.dae output-dir") }
let input = CommandLine.arguments[1]
let outDir = CommandLine.arguments[2]
guard let scene = try? SCNScene(url: URL(fileURLWithPath: input), options: nil) else { fatalError("cannot load scene") }

// The fused KiCad GLB carries a renderer-specific solder-mask channel that can
// survive OBJ import as magenta. Cover only the rendered PCB substrate with a
// visualization-only green slab. Its top is exactly the physical top surface;
// the tiny 10 um X/Z oversize prevents fallback-colored edge pixels without
// changing production geometry. Copper/pads and all component bodies remain
// above the slab.
func addMaskSlab() {
    let top = 0.0012001
    let height = 0.001450
    let skin = SCNBox(width:0.056315, height:height, length:0.062020, chamferRadius:0.00003)
    let material = SCNMaterial()
    material.diffuse.contents = NSColor(calibratedRed:0.025, green:0.32, blue:0.085, alpha:1)
    material.ambient.contents = NSColor(calibratedRed:0.008, green:0.085, blue:0.022, alpha:1)
    material.specular.contents = NSColor(calibratedWhite:0.20, alpha:1)
    material.metalness.contents = 0.0
    material.roughness.contents = 0.48
    material.lightingModel = .blinn
    skin.materials = [material]
    let node = SCNNode(geometry:skin)
    node.name = "VISUALIZATION_SOLDER_MASK_GREEN"
    node.position = SCNVector3(0.056295/2.0, top-height/2.0, 0.062000/2.0)
    scene.rootNode.addChildNode(node)
}
addMaskSlab()

// Optional visualization-only standardized package envelopes. Coordinates are
// read from the current board; no production model or footprint is modified.
if CommandLine.arguments.count >= 4,
   let data = try? Data(contentsOf: URL(fileURLWithPath: CommandLine.arguments[3])),
   let root = try? JSONSerialization.jsonObject(with: data) as? [String:Any],
   let items = root["items"] as? [[String:Any]] {
    let palette:[String:NSColor] = [
        "resistor": NSColor(calibratedRed:0.12,green:0.10,blue:0.08,alpha:1),
        "capacitor": NSColor(calibratedRed:0.72,green:0.60,blue:0.42,alpha:1),
        "ic": NSColor(calibratedWhite:0.035,alpha:1),
        "module": NSColor(calibratedRed:0.58,green:0.60,blue:0.60,alpha:1),
        "diode": NSColor(calibratedWhite:0.08,alpha:1),
        "led": NSColor(calibratedRed:0.75,green:0.08,blue:0.04,alpha:1),
        "inductor": NSColor(calibratedWhite:0.12,alpha:1),
        "fuse": NSColor(calibratedRed:0.86,green:0.83,blue:0.68,alpha:1)
    ]
    for item in items {
        guard let ref=item["reference"] as? String,
              let x=item["x_mm"] as? Double, let z=item["y_mm"] as? Double,
              let rot=item["rotation_deg"] as? Double,
              let dims=item["size_mm"] as? [Double], dims.count==3,
              let kind=item["material"] as? String else { continue }
        let box=SCNBox(width:dims[0]/1000.0, height:dims[2]/1000.0, length:dims[1]/1000.0, chamferRadius:min(dims[0],dims[1],dims[2])/10000.0)
        let mat=SCNMaterial(); mat.diffuse.contents=palette[kind] ?? NSColor.gray; mat.lightingModel = .blinn
        box.materials=[mat]
        let node=SCNNode(geometry:box); node.name="RENDER_ENVELOPE_\(ref)"
        node.position=SCNVector3(x/1000.0, 0.00120+dims[2]/2000.0, z/1000.0)
        node.eulerAngles.y=CGFloat(-rot * Double.pi / 180.0)
        scene.rootNode.addChildNode(node)
    }
}

// Assimp's OBJ conversion preserves material names but SceneKit may not resolve
// the adjacent MTL in a headless process. Reapply its Kd colors explicitly.
let mtlURL = URL(fileURLWithPath: input).deletingPathExtension().appendingPathExtension("mtl")
var colors:[String:NSColor] = [:]
if let src = try? String(contentsOf: mtlURL, encoding:.utf8) {
    var current:String? = nil
    for raw in src.split(separator:"\n") {
        let p = raw.split(separator:" ")
        if p.count >= 2 && p[0] == "newmtl" { current = String(p[1]) }
        if p.count >= 4 && p[0] == "Kd", let n=current,
           let r=Double(p[1]), let g=Double(p[2]), let b=Double(p[3]) {
            colors[n] = NSColor(calibratedRed:r, green:g, blue:b, alpha:1)
        }
    }
}
scene.rootNode.enumerateChildNodes { node, _ in
    if node.name?.hasPrefix("RENDER_ENVELOPE_") == true || node.name == "VISUALIZATION_SOLDER_MASK_GREEN" { return }
    guard let geo=node.geometry else { return }
    let nodeName = node.name ?? ""
    if ProcessInfo.processInfo.environment["SCENE_DEBUG"] == "1" {
        print("NODE", node.name ?? "<nil>", "MATS", geo.materials.map{$0.name ?? "<nil>"}.joined(separator:","))
    }
    for mat in geo.materials {
        let materialName = mat.name ?? ""
        // Assimp may fuse every OBJ group into one SceneKit node, so the
        // board-layer material ids are the reliable semantic discriminator.
        if nodeName.hasPrefix("board_soldermask") || materialName == "mat_16" || materialName == "mat_17" {
            // SceneKit otherwise retains an unresolved GLB texture channel and
            // displays its bright-magenta missing-material fallback.
            mat.diffuse.contents = NSColor(calibratedRed:0.025, green:0.285, blue:0.105, alpha:1)
            mat.ambient.contents = NSColor(calibratedRed:0.012, green:0.075, blue:0.028, alpha:1)
            mat.transparent.contents = NSColor.white
            mat.multiply.contents = NSColor.white
            mat.normal.contents = nil
            mat.reflective.contents = nil
            mat.metalness.contents = 0.0
            mat.roughness.contents = 0.46
            mat.transparency = 1.0
            mat.blendMode = .replace
            mat.writesToDepthBuffer = true
        } else if nodeName.hasPrefix("board_copper") || materialName == "mat_14" {
            mat.diffuse.contents = NSColor(calibratedRed:0.72, green:0.55, blue:0.06, alpha:1)
            mat.ambient.contents = NSColor(calibratedRed:0.16, green:0.10, blue:0.01, alpha:1)
            mat.metalness.contents = 0.72
            mat.roughness.contents = 0.30
        } else if nodeName.hasPrefix("board_silkscreen") || materialName == "mat_15" {
            mat.diffuse.contents = NSColor(calibratedWhite:0.92, alpha:1)
            mat.ambient.contents = NSColor(calibratedWhite:0.20, alpha:1)
            mat.metalness.contents = 0.0
            mat.roughness.contents = 0.58
        } else if nodeName.hasPrefix("board_PCB") || materialName == "mat_18" {
            mat.diffuse.contents = NSColor(calibratedRed:0.10, green:0.16, blue:0.09, alpha:1)
            mat.ambient.contents = NSColor(calibratedRed:0.025, green:0.040, blue:0.022, alpha:1)
            mat.metalness.contents = 0.0
            mat.roughness.contents = 0.72
        } else if let n=mat.name, let c=colors[n] {
            mat.diffuse.contents=c
        } else {
            mat.diffuse.contents=NSColor(calibratedWhite:0.22, alpha:1)
        }
        mat.emission.contents=NSColor.black
        mat.lightingModel = .blinn
        mat.specular.contents = NSColor(calibratedWhite:0.20, alpha:1)
        mat.shininess = 0.18
    }
}

scene.background.contents = NSColor(calibratedWhite: 0.055, alpha: 1)
let lo = SCNVector3(0.0, -0.00005, -0.00602)
let hi = SCNVector3(0.056295, 0.008427, 0.067275)
let center = SCNVector3((lo.x+hi.x)/2, (lo.y+hi.y)/2, (lo.z+hi.z)/2)

func lookAt(_ node: SCNNode, _ target: SCNVector3) {
    node.look(at: target, up: SCNVector3(0,1,0), localFront: SCNVector3(0,0,-1))
}
func light(_ pos: SCNVector3, intensity: CGFloat) {
    let n = SCNNode(); let l = SCNLight(); l.type = .omni; l.intensity = intensity; l.color = NSColor.white
    n.light = l; n.position = pos; scene.rootNode.addChildNode(n)
}
let ambient = SCNNode(); let al = SCNLight(); al.type = .ambient; al.intensity = 120; al.color = NSColor.white
ambient.light = al; scene.rootNode.addChildNode(ambient)
light(SCNVector3(-0.08,0.12,-0.06), intensity: 320)
light(SCNVector3(0.13,0.08,0.12), intensity: 220)

struct View { let name:String; let pos:SCNVector3; let target:SCNVector3; let ortho:Bool; let scale:Double }
let views:[View] = [
    View(name:"opposite-edge-diagnostic", pos:SCNVector3(0.028,0.020,-0.080), target:SCNVector3(0.022,0.003,0.020), ortho:false, scale:0),
    View(name:"full-top-orthographic", pos:SCNVector3(center.x,0.17,center.z), target:center, ortho:true, scale:0.070),
    View(name:"full-bottom-orthographic", pos:SCNVector3(center.x,-0.16,center.z), target:center, ortho:true, scale:0.070),
    View(name:"perspective", pos:SCNVector3(-0.050,0.095,0.135), target:center, ortho:false, scale:0),
    // Board +Y maps to scene +Z. These cameras are outside the lower edge,
    // looking inward toward the connector mouths at Z ~= 0.062 m.
    View(name:"lower-edge-front", pos:SCNVector3(0.019,0.014,0.135), target:SCNVector3(0.019,0.003,0.056), ortho:false, scale:0),
    View(name:"j1-close-up", pos:SCNVector3(0.008,0.013,0.112), target:SCNVector3(0.008,0.003,0.056), ortho:false, scale:0),
    View(name:"j2-close-up", pos:SCNVector3(0.026,0.013,0.112), target:SCNVector3(0.026,0.003,0.056), ortho:false, scale:0),
    View(name:"connector-pad-top-close", pos:SCNVector3(0.017,0.060,0.069), target:SCNVector3(0.017,0.001,0.054), ortho:false, scale:0),
    View(name:"connector-underside-close", pos:SCNVector3(0.017,-0.048,0.068), target:SCNVector3(0.017,0.000,0.054), ortho:false, scale:0),
    View(name:"usb-c-right-edge", pos:SCNVector3(0.105,0.018,0.010), target:SCNVector3(0.052,0.003,0.010), ortho:false, scale:0)
    ,View(name:"side-height", pos:SCNVector3(-0.090,0.020,center.z), target:SCNVector3(0.020,0.003,center.z), ortho:false, scale:0)
    ,View(name:"u1-close-up", pos:SCNVector3(-0.020,0.050,0.005), target:SCNVector3(0.011,0.002,0.010), ortho:false, scale:0)
    ,View(name:"switches-close-up", pos:SCNVector3(0.027,0.090,0.039), target:SCNVector3(0.027,0.002,0.039), ortho:true, scale:0.052)
    ,View(name:"sw1-close-up", pos:SCNVector3(-0.012,0.030,0.029), target:SCNVector3(0.006,0.002,0.0345), ortho:false, scale:0)
    ,View(name:"sw2-close-up", pos:SCNVector3(0.068,0.030,0.039), target:SCNVector3(0.0475,0.002,0.0435), ortho:false, scale:0)
    ,View(name:"d2-close-up", pos:SCNVector3(0.038,0.030,0.027), target:SCNVector3(0.0305,0.001,0.0345), ortho:false, scale:0)
    ,View(name:"l1-close-up", pos:SCNVector3(0.045,0.032,0.018), target:SCNVector3(0.0355,0.0015,0.026), ortho:false, scale:0)
]

let renderer = SCNRenderer(device:nil, options:nil)
renderer.scene = scene
for v in views {
    let camNode = SCNNode(); let cam = SCNCamera(); cam.fieldOfView = 34; cam.zNear = 0.001; cam.zFar = 2
    cam.usesOrthographicProjection = v.ortho; if v.ortho { cam.orthographicScale = v.scale }
    camNode.camera = cam; camNode.position = v.pos; lookAt(camNode, v.target); scene.rootNode.addChildNode(camNode)
    renderer.pointOfView = camNode
    let img = renderer.snapshot(atTime:0, with:CGSize(width:2400,height:1800), antialiasingMode:.multisampling4X)
    var rect = NSRect(origin:.zero, size:img.size)
    guard let cg = img.cgImage(forProposedRect:&rect, context:nil, hints:nil) else { fatalError("cg") }
    let rep = NSBitmapImageRep(cgImage:cg)
    guard let png = rep.representation(using:.png, properties:[:]) else { fatalError("png") }
    try! png.write(to:URL(fileURLWithPath:outDir).appendingPathComponent(v.name+".png"))
    camNode.removeFromParentNode()
}
