{
name: "GEO",
index: "world",
valid_begin: [0,0],
valid_end: [0,0],
mother: "",
type: "box",
size: [500.0, 500.0, 500.0],
position: [0.0, 0.0, 0.0],
material: "scintWithNucleonParams",
color: [1.0, 0.0, 0.0, 0.0],
drawstyle: "solid",
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
pos_table: "PMTINFO_CUBE",
orientation: "manual",
}
