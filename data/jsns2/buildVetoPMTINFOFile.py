from math import sin, cos, pi

outfile = open('PMTINFO_VETO.ratdb', 'w')

#Write all the stuff that goes at the top of the PMTINFO File

outfile.write("""
{
  name:"PMTINFO_VETO",
  valid_begin: [0,0],
  valid_end: [0,0],
""")

#Now we build arrays of all the PMT positions.

#Detector parameters
PMTPosRadius = 2110.0 #in mm
vetoHeight = 3500.0 #in mm
PMTRadius = 64.0 #in mm
numPMTPlaces = 12
PMTOffsetVert = PMTRadius + 120  #in mm; distance between PMT center and tank
PMTOffsetHori = PMTRadius + 100

#PMTs are places in pairs. Need to offset the first from the second
#by some amount so they don't overlap in the geometry. This was
#found by trying different things
secondOffsetAngle = (2*PMTRadius + 20)/PMTPosRadius 

#PMT Position Lists
xPosList = "x: ["
yPosList = "y: ["
zPosList = "z: ["
xDirList = "dir_x: ["
yDirList = "dir_y: ["
zDirList = "dir_z: ["
typeList = "type: ["

#Separation between PMTs within a given ring in the phi-direction
phiSeparation = 2*pi/float(numPMTPlaces)

for PMT in range (0, numPMTPlaces): 
    x = PMTPosRadius*cos(PMT*phiSeparation)
    y = PMTPosRadius*sin(PMT*phiSeparation)
    z = vetoHeight/2.0 - PMTOffsetVert
    xPosList += ' %.4f'%x + ',' + ' %.4f'%x + ',' 
    yPosList += ' %.4f'%y + ',' + ' %.4f'%y + ','
    zPosList += ' %.4f'%z + ',' + ' %.4f'%(-z) + ','
    xDirList += ' %.4f'%(0) + ',' + ' %.4f'%(0) + ','
    yDirList += ' %.4f'%(0) + ',' + ' %.4f'%(0) + ','
    zDirList += ' %.4f'%(-1) + ',' + ' %.4f'%(1) + ','
    typeList += "2, 2, " #type 2 for veto PMTs

for PMT in range (0, numPMTPlaces): 
    x = PMTPosRadius*cos(PMT*phiSeparation+secondOffsetAngle)
    y = PMTPosRadius*sin(PMT*phiSeparation+secondOffsetAngle)
    z = vetoHeight/2.0 - PMTOffsetHori
    xPosList += ' %.4f'%x + ',' + ' %.4f'%x + ',' 
    yPosList += ' %.4f'%y + ',' + ' %.4f'%y + ','
    zPosList += ' %.4f'%z + ',' + ' %.4f'%(-z) + ','
    xDirList += ' %.4f'%(-x) + ',' + ' %.4f'%(-x) + ','
    yDirList += ' %.4f'%(-y) + ',' + ' %.4f'%(-y) + ','
    zDirList += ' %.4f'%(0) + ',' + ' %.4f'%(0) + ','
    typeList += "2, 2, " #type 2 for veto PMTs


#Remove the extra commas and add the closing brackets
xPosList = xPosList[:-1] + "],\n"
yPosList = yPosList[:-1] + "],\n"
zPosList = zPosList[:-1] + "],\n"
xDirList = xDirList[:-1] + "],\n"
yDirList = yDirList[:-1] + "],\n"
zDirList = zDirList[:-1] + "],\n"
typeList = typeList[:-1] + "]\n"

#Now write all the positions to the file
outfile.write(xPosList + yPosList + zPosList + xDirList + yDirList + zDirList + typeList + "}")

outfile.close()
