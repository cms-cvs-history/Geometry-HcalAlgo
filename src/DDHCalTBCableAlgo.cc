///////////////////////////////////////////////////////////////////////////////
// File: DDHCalTBCableAlgo.cc
// Description: Cable mockup between barrel and endcap gap
///////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <algorithm>

namespace std{} using namespace std;
#include "DetectorDescription/Parser/interface/DDLParser.h"
#include "DetectorDescription/Base/interface/DDdebug.h"
#include "DetectorDescription/Base/interface/DDTypes.h"
#include "DetectorDescription/Base/interface/DDutils.h"
#include "DetectorDescription/Core/interface/DDPosPart.h"
#include "DetectorDescription/Core/interface/DDLogicalPart.h"
#include "DetectorDescription/Core/interface/DDSolid.h"
#include "DetectorDescription/Core/interface/DDMaterial.h"
#include "DetectorDescription/Core/interface/DDCurrentNamespace.h"
#include "DetectorDescription/Core/interface/DDSplit.h"
#include "Geometry/HcalAlgo/interface/DDHCalTBCableAlgo.h"
#include "CLHEP/Units/PhysicalConstants.h"
#include "CLHEP/Units/SystemOfUnits.h"

DDHCalTBCableAlgo::DDHCalTBCableAlgo(): theta(0),rmax(0),zoff(0) {
  DCOUT('a', "DDHCalTBCableAlgo info: Creating an instance");
}

DDHCalTBCableAlgo::~DDHCalTBCableAlgo() {}


void DDHCalTBCableAlgo::initialize(const DDNumericArguments & nArgs,
				   const DDVectorArguments & vArgs,
				   const DDMapArguments & ,
				   const DDStringArguments & sArgs,
				   const DDStringVectorArguments & ) {

  genMat      = sArgs["MaterialName"];
  nsectors    = int (nArgs["NSector"]);
  nsectortot  = int (nArgs["NSectorTot"]);
  nhalf       = int (nArgs["NHalf"]);
  rin         = nArgs["RIn"];
  theta       = vArgs["Theta"];
  rmax        = vArgs["RMax"];
  zoff        = vArgs["ZOff"];

  absMat      = sArgs["AbsMatName"];
  thick       = nArgs["Thickness"];
  width1      = nArgs["Width1"];
  length1     = nArgs["Length1"];
  width2      = nArgs["Width2"];
  length2     = nArgs["Length2"];
  gap2        = nArgs["Gap2"];

  DCOUT('A', "DDHCalTBCableAlgo debug: General material " << genMat << "\tSectors "  << nsectors << ", " << nsectortot << "\tHalves " << nhalf << "\tRin " << rin);
  for (unsigned int i = 0; i < theta.size(); i++)
    DCOUT('A', "\t" << i << " Theta " << theta[i] << " rmax " << rmax[i] << " zoff " << zoff[i]);
  DCOUT('A', "\tCable mockup made of " << absMat << "\tThick " << thick << "\tLength and width " << length1 << ", " << width1 <<" and "	<< length2 << ", " << width2 << " Gap " << gap2);

  idName      = sArgs["MotherName"];
  idNameSpace = DDCurrentNamespace::ns();
  rotns       = sArgs["RotNameSpace"];
  DDName parentName = parent().name(); 
  DCOUT('A', "DDHCalTBCableAlgo debug: Parent " << parentName << " idName " << idName << " NameSpace " << idNameSpace << " for solids etc. and " << rotns << " for rotations");
}

void DDHCalTBCableAlgo::execute() {
  
  DCOUT('a', "==>> Constructing DDHCalTBCableAlgo...");
  unsigned int i=0;

  double alpha = pi/nsectors;
  double dphi  = nsectortot*twopi/nsectors;

  double zstep0 = zoff[1]+rmax[1]*tan(theta[1])+(rin-rmax[1])*tan(theta[2]);
  double zstep1 = zstep0+thick/cos(theta[2]);
  double zstep2 = zoff[3];
 
  double rstep0 = rin + (zstep2-zstep1)/tan(theta[2]);
  double rstep1 = rin + (zstep1-zstep0)/tan(theta[2]);

  vector<double> pgonZ;
  pgonZ.push_back(zstep0); 
  pgonZ.push_back(zstep1);
  pgonZ.push_back(zstep2); 
  pgonZ.push_back(zstep2+thick/cos(theta[2])); 

  vector<double> pgonRmin;
  pgonRmin.push_back(rin); 
  pgonRmin.push_back(rin);
  pgonRmin.push_back(rstep0); 
  pgonRmin.push_back(rmax[2]); 

  vector<double> pgonRmax;
  pgonRmax.push_back(rin); 
  pgonRmax.push_back(rstep1); 
  pgonRmax.push_back(rmax[2]); 
  pgonRmax.push_back(rmax[2]); 

  string name("Null");
  DDSolid solid;
  solid = DDSolidFactory::polyhedra(DDName(idName, idNameSpace),
				    nsectortot, -alpha, dphi, pgonZ, 
				    pgonRmin, pgonRmax);
  DCOUT('a', "DDHCalTBCableAlgo test: " << DDName(idName, idNameSpace) << " Polyhedra made of " << genMat << " with " << nsectortot << " sectors from " << -alpha/deg << " to " << (-alpha+dphi)/deg << " and with " << pgonZ.size() << " sections");
  for (i = 0; i <pgonZ.size(); i++) 
    DCOUT('a', "\t\tZ = " << pgonZ[i] << "\tRmin = " << pgonRmin[i] << "\tRmax = " << pgonRmax[i]);
  
  DDName matname(DDSplit(genMat).first, DDSplit(genMat).second); 
  DDMaterial matter(matname);
  DDLogicalPart genlogic(solid.ddname(), matter, solid);

  DDName parentName = parent().name(); 
  DDTranslation r0(0.0, 0.0, 0.0);
  DDRotation rot;
  DDpos(DDName(idName, idNameSpace), parentName, 1, r0, rot);
  DCOUT('a', "DDHCalTBCableAlgo test: " << DDName(idName, idNameSpace) << " number 1 positioned in " << parentName << " at " << r0 << " with " << rot);
  
  if (nhalf != 1) {
    rot = DDRotation(DDName("180D", rotns));
    DDpos (DDName(idName, idNameSpace), parentName, 2, r0, rot);
    DCOUT('a', "DDHCalTBCableAlgo test: " << DDName(idName, idNameSpace) << " number 2 positioned in " << parentName  << " at " << r0 << " with " << rot);
  } 
  
  //Construct sector (from -alpha to +alpha)
  name = idName + "Module";
  DCOUT('a', "DDHCalTBCableAlgo test: " << DDName(name, idNameSpace) << " Polyhedra made of " << genMat << " with 1 sector from " << -alpha/deg << " to " << alpha/deg << " and with " << pgonZ.size() << " sections");
  for (i = 0; i < pgonZ.size(); i++) 
    DCOUT('a', "\t\tZ = " << pgonZ[i] << "\tRmin = " << pgonRmin[i] << "\tRmax = " << pgonRmax[i]);
  solid =   DDSolidFactory::polyhedra(DDName(name, idNameSpace),
				      1, -alpha, 2*alpha, pgonZ,
				      pgonRmin, pgonRmax);
  DDLogicalPart seclogic(solid.ddname(), matter, solid);
  
  for (int ii=0; ii<nsectortot; ii++) {
    double phi    = ii*2*alpha;
    double phideg = phi/deg;
    
    DDRotation rotation;
    string rotstr("NULL");
    if (phideg != 0) {
      rotstr = "R"; 
      if (phideg < 100)	rotstr = "R0"; 
      rotstr = rotstr + dbl_to_string(phideg);
      rotation = DDRotation(DDName(rotstr, rotns)); 
      if (!rotation) {
	DCOUT('a', "DDHCalTBCableAlgo test: Creating a new rotation " << rotstr << "\t" << 90 << "," << phideg << "," << 90 << "," << (phideg+90) << ","  << 0  << "," <<  0);
	rotation = DDrot(DDName(rotstr, idNameSpace), 90*deg, phideg*deg, 
			 90*deg, (90+phideg)*deg, 0*deg,  0*deg);
      } //if !rotation
    } //if phideg!=0
  
    DDpos (seclogic, genlogic, ii+1, DDTranslation(0.0, 0.0, 0.0), rotation);
    DCOUT('a', "DDHCalTBCableAlgo test: " << seclogic.name() << " number " << ii+1 << " positioned in " << genlogic.name() << " at (0,0,0) with " << rotation);
  }
  
  //Now a trapezoid of air
  double rinl  = pgonRmin[0] + thick * sin(theta[2]);
  double routl = pgonRmax[2] - thick * sin(theta[2]);
  double dx1   = rinl * tan(alpha);
  double dx2   = 0.90 * routl * tan(alpha);
  double dy    = 0.50 * thick;
  double dz    = 0.50 * (routl -rinl);
  name  = idName + "Trap";
  solid = DDSolidFactory::trap(DDName(name, idNameSpace), dz, 0, 0, dy, dx1, 
			       dx1, 0, dy, dx2, dx2, 0);
  DCOUT('a', "DDHCalTBCableAlgo test: " << solid.name() <<" Trap made of " << genMat << " of dimensions " << dz << ", 0, 0, " << dy << ", " << dx1 << ", " << dx1 << ", 0, " << dy << ", " << dx2 << ", "  << dx2 << ", 0");
  DDLogicalPart glog(solid.ddname(), matter, solid);

  string rotstr = name;
  DCOUT('a', "DDHCalTBCableAlgo test: Creating a new rotation: " << rotstr << "\t" << 90 << "," << 270 << "," << (180-theta[2]/deg) << "," << 0 << "," << (90-theta[2]/deg) << "," << 0);
  rot = DDrot(DDName(rotstr, idNameSpace), 90*deg, 270*deg, 
	      180*deg-theta[2], 0*deg, 90*deg-theta[2], 0*deg);
  DDTranslation r1(0.5*(rinl+routl), 0, 0.5*(pgonZ[1]+pgonZ[2]));
  DDpos(glog, seclogic, 1, r1, rot);
  DCOUT('a', "DDHCalTBCableAlgo test: " << glog.name() << " number 1 positioned in "  << seclogic.name() << " at " << r1 << " with " << rot);

  //Now the cable of type 1
  name = idName + "Cable1";
  double phi  = atan((dx2-dx1)/(2*dz));
  double xmid = 0.5*(dx1+dx2);
  solid = DDSolidFactory::box(DDName(name, idNameSpace), 0.5*width1,
			      0.5*thick, 0.5*length1);
  DCOUT('a', "DDHCalTBCableAlgo test: " << solid.name() << " Box made of " << absMat << " of dimension " << 0.5*width1 << ", " << 0.5*thick << ", " << 0.5*length1);
  DDName absname(DDSplit(absMat).first, DDSplit(absMat).second); 
  DDMaterial absmatter(absname);
  DDLogicalPart cablog1(solid.ddname(), absmatter, solid);

  rotstr = idName + "Left";
  DCOUT('a', "DDHCalTBCableAlgo test: Creating a new rotation " << rotstr << "\t"  << (90+phi/deg) << "," << 0  << "," << 90 << "," << 90 << "," << phi/deg << "," << 0);
  DDRotation rot2 = DDrot(DDName(rotstr, idNameSpace), 90*deg+phi, 0*deg, 
			  90*deg, 90*deg, phi, 0*deg);
  DDTranslation r2((xmid-0.5*width1*cos(phi)), 0, 0);
  DDpos(cablog1, glog, 1, r2, rot2);
  DCOUT('a', "DDHCalTBCableAlgo test: " << cablog1.name() << " number 1 positioned in "  << glog.name() << " at " << r2	<< " with " << rot2);

  rotstr = idName + "Right";
  DCOUT('a', "DDHCalTBCableAlgo test: Creating a new rotation " << rotstr << "\t"  << (90-phi/deg) << "," << 0  << "," << 90 << "," << 90 << "," << -phi/deg << "," << 0);
  DDRotation rot3 = DDrot(DDName(rotstr, idNameSpace), 90*deg-phi, 0*deg, 
			  90*deg, 90*deg, -phi, 0*deg);
  DDTranslation r3(-(xmid-0.5*width1*cos(phi)), 0, 0);
  DDpos(cablog1, glog, 2, r3, rot3);
  DCOUT('a', "DDHCalTBCableAlgo test: " << cablog1.name() << " number 2 positioned in "  << glog.name() << " at " << r3 << " with " << rot3);

  //Now the cable of type 2
  name = idName + "Cable2";
  solid = DDSolidFactory::box(DDName(name, idNameSpace), 0.5*width2,
			      0.5*thick, 0.5*length2);
  DCOUT('a', "DDHCalTBCableAlgo test: " << solid.name() << " Box made of " << absMat << " of dimension " << 0.5*width2 << ", " << 0.5*thick << ", " << 0.5*length2);
  DDLogicalPart cablog2(solid.ddname(), absmatter, solid);

  double xpos = 0.5*(width2+gap2);
  DDpos (cablog2, glog, 1, DDTranslation(xpos, 0.0, 0.0), DDRotation());
  DCOUT('a', "DDHCalTBCableAlgo test: " << cablog2.name() << " number 1 positioned in "  << glog.name() << " at (" << xpos << "," << 0 << "," << 0 << ") with no rotation");
  DDpos (cablog2, glog, 2, DDTranslation(-xpos, 0.0, 0.0), DDRotation());
  DCOUT('a', "DDHCalTBCableAlgo test: " << cablog2.name() << " number 2 positioned in "  << glog.name() << " at (" <<-xpos << "," << 0 << "," << 0 << ") with no rotation");

  DCOUT('a', "<<== End of DDHCalTBCableAlgo construction ...");
}
