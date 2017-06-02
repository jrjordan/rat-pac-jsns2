from math import sin, cos, pi

outfile = open('PMTINFO.ratdb', 'w')

#Write all the stuff that goes at the top of the PMTINFO File

outfile.write("""
{
  name:"PMTINFO",
  valid_begin: [0,0],
  valid_end: [0,0],
""")

#Now we build arrays of all the PMT positions.

#Detector parameters
tankRadius = 1630.0 #in mm
tankHeight = 2560.0 #in mm
PMTRadius = 101.0 #in mm
PMTOffset = 250.0 #in mm; distance between PMT center and tank

#PMT Position Lists
xPosList = "x: ["
yPosList = "y: ["
zPosList = "z: ["
xDirList = "dir_x: ["
yDirList = "dir_y: ["
zDirList = "dir_z: ["
typeList = "type: ["

#Barrel PMT Positions

numRings = 5
PMTsPerRing = 24
ringRadius = tankRadius + PMTOffset #in mm

#Separation between rings of PMTs in the z-direction
#Subtract twice the PMT radius from the height to prevent PMT overhang
#above or below the detector
ringSeparation = (tankHeight - 2.0*PMTRadius)/(float(numRings) - 1.0)

#Separation between PMTs within a given ring in the phi-direction
phiSeparation = 2*pi/float(PMTsPerRing)

for ring in range(0, numRings):
    z = -tankHeight/2.0 + PMTRadius + ring*ringSeparation
    for ringPMT in range(0, PMTsPerRing):
        x = ringRadius*cos(ringPMT*phiSeparation)
        y = ringRadius*sin(ringPMT*phiSeparation)
        xPosList += ' %.4f'%x + ','
        yPosList += ' %.4f'%y + ','
        zPosList += ' %.4f'%z + ','
        xDirList += ' %.4f'%(-x) + ','
        yDirList += ' %.4f'%(-y) + ','
        zDirList += ' %.4f'%(0) + ','
        typeList += "1, "

#Top and Bottom PMT Positions
#PMTs are placed in concentric rings on the top and bottom

numRings = 3
numPMTsInnerRing = 6

#Separation between top and bottom rings in the radial coordinate
ringSeparation = (tankRadius - PMTRadius)/float(numRings)

for ring in range(1, numRings+1):
    ringRadius = ring*ringSeparation
    numPMTs = ring*numPMTsInnerRing
    phiSeparation = 2*pi/float(numPMTs)
    for ringPMT in range(0, numPMTs):
        x = ringRadius*cos(ringPMT*phiSeparation)
        y = ringRadius*sin(ringPMT*phiSeparation)
        z = tankHeight/2.0 + PMTOffset
        xPosList += ' %.4f'%x + ',' + ' %.4f'%x + ',' 
        yPosList += ' %.4f'%y + ',' + ' %.4f'%y + ','
        zPosList += ' %.4f'%z + ',' + ' %.4f'%(-z) + ','
        xDirList += ' %.4f'%(0) + ',' + ' %.4f'%(0) + ','
        yDirList += ' %.4f'%(0) + ',' + ' %.4f'%(0) + ','
        zDirList += ' %.4f'%(-1) + ',' + ' %.4f'%(1) + ','
        typeList += "1, "

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
