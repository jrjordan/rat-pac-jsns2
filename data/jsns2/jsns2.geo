{
name: "GEO",
index: "world",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "",
type: "box",
size: [3000.0, 3000.0, 3000.0] 
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
color: [0.0, 0.5, 0.5, 1.0],
drawstyle: "wireframe"
}

//Veto scintillator volume

{
name: "GEO",
index: "veto",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "stainlessTank",
type: "tube",
r_max: 2300.0,
size_z: 1750.0,
position: [0.0, 0.0, 0.0],
material: "scintLABPPOBisMSB",
color: [0.0, 0.5, 0.5, 0.4],
drawstyle: "solid"
}

//Region between the veto and the buffer. Used
//put mirror and black sheet on the two sides
//appropriately

{
name: "GEO",
index: "opticalSeparator",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "veto",
type: "tube",
r_max: 1883.0,
size_z: 1533.0,
position: [0.0, 0.0, 0.0],
material: "mirror",
color: [0.0, 0.0, 1.0, 1.0],
drawstyle: "wireframe"
}

//buffer region

{
name: "GEO",
index: "buffer",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "opticalSeparator",
type: "tube",
r_max: 1880.0,
size_z: 1530.0,
position: [0.0, 0.0, 0.0],
material: "scintLABPPOBisMSB",
color: [0.0, 0.0, 1.0, 0.6],
drawstyle: "solid"
}

//3 cm thick acrylic tank

{
name: "GEO",
index: "acrylicTank",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "buffer", 
type: "tube",
r_max: 1600.0,
size_z: 1280.0,
position: [0.0, 0.0, 0.0],
material: "acrylic_uvt",
color: [1.0, 0.0, 0.0, 1.0],
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
r_max: 1570.0,
size_z: 1250.0,
position: [0.0, 0.0, 0.0],
material: "scintLABPPOBisMSBGd",
color: [1.0, 0.0, 0.0, 0.8],
drawstyle: "solid"
}

//Black sheet to prevent reflections
//and transmission to the veto

{
name: "GEO",
index: "blackSheet",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "border",
type: "border",
volume1: "opticalSeparator", 
volume2: "buffer",
surface: "blackNylon",
reverse: 1,
}

{
name: "GEO",
index: "innerMirror",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "border",
type: "border",
volume1: "opticalSeparator",
volume2: "veto",
surface: "ptfe",
reverse: 1,
}

{
name: "GEO",
index: "outerMirror",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "border",
type: "border",
volume1: "stainlessTank",
volume2: "veto",
surface: "ptfe",
reverse: 1,
}

//Target region PMTs

{
name: "GEO",
index: "innerPMTs",
enable: 1,
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "buffer", //buffer
type: "pmtarray",
//pmt_model: "r5912",
pmt_model: "r7081",
pmt_detector_type: "idpmt",
sensitive_detector: "/mydet/pmt/inner",
efficiency_correction: 1.000,
pos_table: "PMTINFO",
orientation: "manual",
}

{
name: "GEO",
index: "vetoPMTs",
enable: 1,
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "veto", //veto
type: "pmtarray",
pmt_model: "r6594",
pmt_detector_type: "idpmt",
sensitive_detector: "/mydet/pmt/inner",
efficiency_correction: 1.000,
pos_table: "PMTINFO_VETO",
orientation: "manual",
}
