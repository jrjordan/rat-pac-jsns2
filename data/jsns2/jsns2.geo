{
name: "GEO",
index: "world",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "",
type: "tube",
r_max: 2500.0,
size_z: 2000.0,
position: [0.0, 0.0, 0.0],
material: "cryostat_vacuum",
color: [1.0, 0.0, 0.0, 0.0],
drawstyle: "wireframe"
}

//5 mm thick stainless tank

{
name: "GEO",
index: "stainlessTank",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "world",
type: "tube",
r_max: 2305.0,
size_z: 1755.0,
position: [0.0, 0.0, 0.0],
material: "stainless_steel",
color: [1.0, 0.0, 0.0, 0.0],
drawstyle: "wireframe"
}

//Veto scintillator volume

{
name: "GEO",
index: "outerScintillator",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "stainlessTank",
type: "tube",
r_max: 2300.0,
size_z: 1750.0,
position: [0.0, 0.0, 0.0],
material: "scintLABPPOBisMSB",
color: [0.0, 1.0, 0.0, 0.2],
drawstyle: "wireframe"
}

//3 cm thick acrylic tank

{
name: "GEO",
index: "acrylicTank",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "outerScintillator",
type: "tube",
r_max: 1630.0,
size_z: 1280.0,
position: [0.0, 0.0, 0.0],
material: "acrylic_uvt",
color: [0.0, 0.0, 1.0, 0.2],
drawstyle: "wireframe"
}

//Target scintillator volume

{
name: "GEO",
index: "innerScintillator",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "acrylicTank",
type: "tube",
r_max: 1600.0,
size_z: 1250.0,
position: [0.0, 0.0, 0.0],
material: "scintLABPPOBisMSB",
color: [1.0, 0.0, 0.0, 0.2],
drawstyle: "wireframe"
}

//Black sheet to prevent reflections

{
name: "GEO",
index: "blackSheet",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "border",
type: "border",
volume1: "stainlessTank",
volume2: "outerScintillator",
surface: "blackNylon",
reverse: 1,
}

//PMTs looking at the inner layer

{
name: "GEO",
index: "innerPMTs",
enable: 1,
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "outerScintillator",
type: "pmtarray",
pmt_model: "r7081",
pmt_detector_type: "idpmt",
sensitive_detector: "/mydet/pmt/inner",
efficiency_correction: 1.000,
pos_table: "PMTINFO",
orientation: "manual",
}
