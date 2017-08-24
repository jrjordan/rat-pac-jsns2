{
name: "GEO",
index: "world",
valid_begin: [0,0],
valid_end: [0,0],
mother: "",
type: "box",
size: [500.,500,500],
position: [0.0, 0.0, 0.0],
material: "cryostat_vacuum",
color: [1.0, 0.0, 0.0, 0.0],
drawstyle: "surface",
}

{
name: "GEO",
index: "steelbox",
valid_begin: [0,0],
valid_end: [0,0],
mother: "world", //world
type: "box",
size: [50.,50.,50.],
position: [0.0, 0.0, 0.0],
material: "stainless_steel", 
color: [1.0, 0.0, 0.0, 1.0],
drawstyle: "surface",
}

{
name: "GEO",
index: "sheet",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "border",
type: "border",
volume1: "steelbox", 
volume2: "world",   
surface: "testMirror",
//surface: "testAbsorber",
reverse: 1,
}

{
name: "GEO",
index: "pmt",
enable: 1,
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "world",
type: "pmtarray",
pmt_model: "RUTCustomPMTLarge", 
pmt_detector_type: "idpmt",
sensitive_detector: "/mydet/pmt/inner",
efficiency_correction: 1.000,
pos_table: "PMTINFO_REFLECT_TEST",
orientation: "manual",
}
