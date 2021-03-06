
{
name: "GEO",
index: "world",
valid_begin: [0,0],
valid_end: [0,0],
mother: "",
type: "sphere",
r_max: 530.,
position: [0.0, 0.0, 0.0],
material: "scintWithWLS",
color: [1.0, 0.0, 0.0, 0.0],
drawstyle: "surface",
}

{
name: "GEO",
index: "scint",
valid_begin: [0,0],
valid_end: [0,0],
mother: "world",
type: "sphere",
r_max: 500.,
position: [0.0, 0.0, 0.0],
material: "scintWithWLS",
color: [1.0, 0.0, 0.0, 0.0],
drawstyle: "surface",
}

{
name: "GEO",
index: "blackSheet",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "border",
type: "border",
volume1: "scint",
volume2: "world",
surface: "blackNylon",
reverse: 1,
}
{
name: "GEO",
index: "pmt",
enable: 1,
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "scint",
type: "pmtarray",
pmt_model: "RUTCustomPMT",
pmt_detector_type: "idpmt",
sensitive_detector: "/mydet/pmt/inner",
efficiency_correction: 1.000,
pos_table: "PMTINFO_SPHERE",
orientation: "manual",
}
