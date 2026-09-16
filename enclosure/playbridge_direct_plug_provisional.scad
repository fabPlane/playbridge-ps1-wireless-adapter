// PROVISIONAL ONLY — NOT MECHANICALLY VERIFIED OR PRINT-READY.
// Reference meshes remain unscaled. The printed keyed shell must align nine
// bare PCB-mounted male pins and transfer mating loads away from solder joints.

reference_top = "reference/BlueRetro2_PS1_top_Ali.stl";
reference_bottom = "reference/BlueRetro2_PS1_bottom_Ali.stl";

pcb_x = 90;
pcb_y = 70;
pcb_z = 1.6;

// Board-envelope design parameters; validate after component-height measurement.
board_clearance_xy = 0.75;
wall = 2.0;
provisional_body_x = pcb_x + 2 * (board_clearance_xy + wall); // 85 mm
provisional_body_y = pcb_y + 2 * (board_clearance_xy + wall); // 65 mm

// Connector-specific values: deliberately undefined until measured.
TBD_nose_width = undef;
TBD_nose_height = undef;
TBD_shoulder_width = undef;
TBD_shoulder_height = undef;
TBD_shoulder_thickness = undef;
provisional_pin_projection = 8.0;
TBD_rear_body_depth = undef;
TBD_contact_centerline_z = undef;
provisional_pin_width = 0.64;
provisional_pin_thickness = 0.64;
TBD_pin_mpn = undef;
TBD_pin_tail_retention_geometry = undef;
TBD_production_drill = undef;
TBD_pin_assembly_method = undef;
TBD_console_fascia_clearance = undef;
TBD_adjacent_port_clearance = undef;
TBD_key_profile = undef;
measured_contact_pitch = 4.0;
measured_first_to_last_span = 32.0;
TBD_case_capture_clearance = undef;

module verified_reference_top() { import(reference_top, convexity=10); }
module verified_reference_bottom() { import(reference_bottom, convexity=10); }

module keyed_pin_shell_tbd() {
    assert(is_num(TBD_nose_width), "Measure controller-plug nose width");
    assert(is_num(TBD_shoulder_width), "Measure shoulder before creating load stops");
    assert(is_num(TBD_contact_centerline_z), "Measure pin/PCB vertical relationship");
    assert(is_string(TBD_pin_mpn), "Select coupon-tested male pin MPN");
    assert(is_num(TBD_pin_tail_retention_geometry), "Obtain pin tail/retention drawing");
    assert(is_num(TBD_production_drill), "Set drill from selected pin drawing");
    assert(is_string(TBD_pin_assembly_method), "Define pin assembly process and fixture");
    // Implement only after all TBD values are populated and fit-checked.
}

// Product axis, fixed independently of the still-unmeasured dimensions:
// CONSOLE SIDE -> [8 mm provisional male-pin projection][keyed shell][J1/PCB]
// PCB/enclosure then extends toward USER SIDE -> [outward-facing USB-C mouth].
// J1 is provisional until pin MPN, tail, drill and assembly method pass coupon testing.
module provisional_male_pin_assembly_tbd() {
    assert(is_num(TBD_nose_width), "PROVISIONAL PLUG: measure nose width");
    assert(is_num(TBD_nose_height), "PROVISIONAL PLUG: measure nose height");
    assert(is_num(TBD_rear_body_depth), "Measure rear pin-retention/PCB cavity");
    assert(is_num(TBD_key_profile), "Digitize keyed mating profile");
    assert(is_num(TBD_case_capture_clearance), "Fit-test enclosure shoulder capture");
    keyed_pin_shell_tbd();
}

// Preview the untouched open-source reference halves only. Do not scale these.
color([0.75,0.85,1.0,0.35]) verified_reference_bottom();
translate([0,0,8]) color([0.75,0.85,1.0,0.20]) verified_reference_top();
