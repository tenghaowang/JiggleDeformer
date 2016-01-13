#include  "JiggleDeformer.h"

MTypeId jiggleDeformer::nodeID(0x00000721);
MObject jiggleDeformer::mTime;
MObject jiggleDeformer::mStiffness;
MObject jiggleDeformer::mDamping;
MObject jiggleDeformer::mJiggleMap;
MObject jiggleDeformer::mStiffnessMap;
MObject jiggleDeformer::mDampingMap;

//constructor
jiggleDeformer::jiggleDeformer(){

}
//destructor
jiggleDeformer::~jiggleDeformer(){

}

MStatus jiggleDeformer::deform(MDataBlock& pDataBlock,MItGeometry& ItGeom,const MMatrix& localToWorldMatrix,unsigned int geomIndex){
	MStatus status;

	MDataHandle timeHandle = pDataBlock.inputValue(mTime);
	MTime currentTime = timeHandle.asTime();

	MDataHandle stiffnessHandle = pDataBlock.inputValue(mStiffness);
	float stiffness = stiffnessHandle.asFloat();

	MDataHandle dampingHandle = pDataBlock.inputValue(mDamping);
	float damping = dampingHandle.asFloat();

	

}

void* jiggleDeformer::nodeCreator(){
	return new jiggleDeformer();

}

MStatus jiggleDeformer::nodeInitialize(){
	MFnMatrixAttribute mAttr;
	MFnNumericAttribute nAttr;
	MFnUnitAttribute uAttr;
	MFnCompoundAttribute cAttr;

	MStatus status;

	mTime = uAttr.create("time", "time", MFnUnitAttribute::kTime, 0.0, &status);
	addAttribute(mTime);
	attributeAffects(mTime, outputGeom);

	mStiffness = nAttr.create("stiffness", "stiffness", MFnNumericData::kFloat, 0.0, &status);
	nAttr.setMin(0.0);
	nAttr.setMax(1.0);
	nAttr.setKeyable(true);
	addAttribute(mStiffness);
	attributeAffects(mStiffness, outputGeom);

	mDamping = nAttr.create("damping", "damping", MFnNumericData::kFloat, 0.0, &status);
	nAttr.setMin(0.0);
	nAttr.setMax(1.0); 
	mAttr.setKeyable(true);
	addAttribute(mDamping);
	attributeAffects(mStiffness, outputGeom);
	
	//jiggleMap
	mJiggleMap = nAttr.create("jiggleMap", "jiggleMap", MFnNumericData::kFloat, 0.0, &status);
	nAttr.setMin(0.0);
	nAttr.setMax(1.0);
	//Sets this attibute should have an array of data.
	//Sets the data for this array from the data in the builder object.
	nAttr.setArray(true);
	nAttr.setUsesArrayDataBuilder(true);
	
	//stiffnessMap
	mStiffnessMap = nAttr.create("stiffnessMap", "stiffnessMap", MFnNumericData::kFloat, 1.0, &status);
	nAttr.setMin(0.0);
	nAttr.setMax(1.0);
	nAttr.setArray(true);
	nAttr.setUsesArrayDataBuilder(true);
	//damingMap
	mDampingMap = nAttr.create("dampingMap", "dampingMap", MFnNumericData::kFloat, 1.0, &status);
	nAttr.setMin(0.0);
	nAttr.setMax(1.0);
	nAttr.setArray(true);
	nAttr.setUsesArrayDataBuilder(true);

	return MS::kSuccess;
}







