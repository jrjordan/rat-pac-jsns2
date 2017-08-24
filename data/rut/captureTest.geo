{
name: "GEO",
index: "world",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "",
type: "box",
size: [2000.0, 2000.0, 2000.0] 
position: [0.0, 0.0, 0.0],
material: "cryostat_vacuum",
color: [1.0, 0.0, 0.0, 0.0],
drawstyle: "wireframe"
}


{
name: "GEO",
index: "scint",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "world",
type: "box",
size: [500.0, 500.0, 500.0] 
position: [0.0, 0.0, 0.0], 
material: "simpleScintWithGd", //simpleScint or simpleScintWithGd
color: [1.0, 0.0, 0.0, 1.0],
drawstyle: "wireframe"
}
