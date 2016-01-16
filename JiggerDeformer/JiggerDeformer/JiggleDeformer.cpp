#include  "JiggleDeformer.h"

MTypeId jiggleDeformer::nodeID(0x00000721);
MObject jiggleDeformer::mTime;
MObject jiggleDeformer::mStiffness;
MObject jiggleDeformer::mDamping;
MObject jiggleDeformer::mScale;
MObject jiggleDeformer::mMaxDisplacement;
MObject jiggleDeformer::mBiasDirection;
MObject jiggleDeformer::mJiggleMap;
MObject jiggleDeformer::mStiffnessMap;
MObject jiggleDeformer::mDampingMap;
MObject jiggleDeformer::mPerGeometry;
MObject jiggleDeformer::mWorldMatrix;


//constructor
jiggleDeformer::jiggleDeformer(){

}
//destructor
jiggleDeformer::~jiggleDeformer(){

}

MStatus jiggleDeformer::jumpToElement(MArrayDataHandle& dataArray, unsigned int index){
	MStatus status;
	status = dataArray.jumpToElement(index);
	if MFAIL(status){
		MArrayDataBuilder dataBuilder = dataArray.builder(&status);
		CHECK_MSTATUS_AND_RETURN_IT(status);
		dataBuilder.addElement(index,&status);
		CHECK_MSTATUS_AND_RETURN_IT(status);
		status = dataArray.set(dataBuilder);
		CHECK_MSTATUS_AND_RETURN_IT(status)
		status = dataArray.jumpToElement(index);
		CHECK_MSTATUS_AND_RETURN_IT(status);
	}
	return status;
}

MStatus jiggleDeformer::getInputMesh(MDataBlock pDataBlock, unsigned int geomIndex, MObject* inputMesh){
	MStatus status;
	MArrayDataHandle inputArrayHandle = pDataBlock.outputArrayValue(input, &status);
	status = jumpToElement(inputArrayHandle, geomIndex);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	MDataHandle inputGeomHandle = inputArrayHandle.inputValue();
	*inputMesh = inputGeomHandle.asMesh();
	return status;
}


MStatus jiggleDeformer::setDependentsDirty(const MPlug& plug, MPlugArray& plugArray){
	MStatus status;
	if (plug == mStiffnessMap || plug == mDampingMap || plug == mJiggleMap ||plug == weights){
		unsigned int geomIndex = 0;
		//if plug belongs to mStiffnessMap, mDampingMap, mJiggleMap
		if (plug.isArray()){
			//plug.parent() returns the parent compound plug
			geomIndex = plug.parent().logicalIndex();
		}
		else{
			geomIndex = plug.array().parent().logicalIndex();
		}
		_dirtyMap[geomIndex] = true;
	}
	return status;
}

MStatus jiggleDeformer::deform(MDataBlock& pDataBlock,MItGeometry& ItGeom,const MMatrix& localToWorldMatrix,unsigned int geomIndex){
	MStatus status;
	MPointArray points;
	ItGeom.allPositions(points);
	MDataHandle timeHandle = pDataBlock.inputValue(mTime);
	MTime currentTime = timeHandle.asTime();
	MDataHandle envHandle = pDataBlock.inputValue(envelope);
	float env = envHandle.asFloat();

	MDataHandle stiffnessHandle = pDataBlock.inputValue(mStiffness);
	float stiffness = stiffnessHandle.asFloat();

	MDataHandle dampingHandle = pDataBlock.inputValue(mDamping);
	float damping = dampingHandle.asFloat();

	MDataHandle scaleHandle = pDataBlock.inputValue(mScale);
	float scale = scaleHandle.asFloat();

	MDataHandle maxdisplacementHandle = pDataBlock.inputValue(mMaxDisplacement);
	float maxdiplacement = maxdisplacementHandle.asFloat()*scale;

	MDataHandle biasdirectionHandle = pDataBlock.inputValue(mBiasDirection);
	float biasdirection = biasdirectionHandle.asFloat();
	
	MMatrix worldtoLocalMatrix = localToWorldMatrix.inverse();
	MFloatVectorArray normals;
	if (!biasdirection == 0.0f){
		MObject inputMesh;
		status = getInputMesh(pDataBlock, geomIndex, &inputMesh);
		CHECK_MSTATUS_AND_RETURN_IT(status);
		MFnMesh mfnMesh(inputMesh, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);
		status = mfnMesh.getVertexNormals(false, normals, MSpace::kObject);
		CHECK_MSTATUS_AND_RETURN_IT(status);
	}



	//Algotithm
	MPointArray& currentPointsPos = _currentPointPos[geomIndex];
	MPointArray& previousPointsPos = _previousPointsPos[geomIndex];
	MTime& previousTime = _PreviousTime[geomIndex];
	if (!_initialized[geomIndex])
	{
		previousTime = currentTime;
		_initialized[geomIndex] = true;
		currentPointsPos.setLength(ItGeom.count());
		previousPointsPos.setLength(ItGeom.count());
		for (unsigned i = 0; i < points.length(); i++)
		{
			currentPointsPos[i] = points[i] * localToWorldMatrix;
			previousPointsPos[i] = currentPointsPos[i];
		}
	}

	//Check if the timestep is just 1 frame, need a stable simulation
	MTime timedifference = currentTime.value() - previousTime.value();
	if (timedifference > 1.0 || timedifference < 0.0)
	{
		_initialized[geomIndex] = false;
		previousTime = currentTime;
		return MStatus::kSuccess;
	}

	MArrayDataHandle mGeometryHandle = pDataBlock.inputArrayValue(mPerGeometry);
	jumpToElement(mGeometryHandle,geomIndex);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	MDataHandle mPerGeometryHandle = mGeometryHandle.inputValue();
	MDataHandle mWorldMatrixHandle = mPerGeometryHandle.child(mWorldMatrix);
	MMatrix WorldMatrix = mWorldMatrixHandle.asMatrix();

	MFloatArray& weights = _weights[geomIndex];
	MFloatArray& stiffnessMap = _stiffnessMap[geomIndex];
	MFloatArray& dampingMap = _dampingMap[geomIndex];
	MFloatArray& jiggleMap = _jiggleMap[geomIndex];
	MIntArray& membershipMap = _membership[geomIndex];

	if (_dirtyMap[geomIndex] || ItGeom.count() != membershipMap.length())
	{
		MArrayDataHandle mJiggleMapHandle = mPerGeometryHandle.child(mJiggleMap);
		MArrayDataHandle mDampingMapHandle = mPerGeometryHandle.child(mDampingMap);
		MArrayDataHandle mStiffnessMapHandle = mPerGeometryHandle.child(mStiffnessMap);
		//read the paintmaps
		jiggleMap.setLength(ItGeom.count());
		dampingMap.setLength(ItGeom.count());
		stiffnessMap.setLength(ItGeom.count());
		weights.setLength(ItGeom.count());

		int ii = 0;
		while (!ItGeom.isDone()){
			//jiggleMap
			status = jumpToElement(mJiggleMapHandle, ItGeom.index());
			CHECK_MSTATUS_AND_RETURN_IT(status);
			jiggleMap[ii] = mJiggleMapHandle.inputValue().asFloat();

			//dampingMap
			status = jumpToElement(mDampingMapHandle, ItGeom.index());
			CHECK_MSTATUS_AND_RETURN_IT(status);
			dampingMap[ii] = mDampingMapHandle.inputValue().asFloat();

			//stiffnessMap
			status = jumpToElement(mStiffnessMapHandle, ItGeom.index());
			CHECK_MSTATUS_AND_RETURN_IT(status);
			stiffnessMap[ii] = mStiffnessMapHandle.inputValue().asFloat();

			//weightMap
			weights[ii] = weightValue(pDataBlock, geomIndex, ItGeom.index());

			//membership
			membershipMap[ii] = ItGeom.index();
			ii++;
			ItGeom.next();
		}
		_dirtyMap[geomIndex] = false;
	}

	MPoint goalPoint, newPoint;
	MVector velocity, goalForce, displacement;
	float dampingMagnitude, stiffnessMagnitude;
	//on the calculations are under worldMatrix
	for (int i = 0; i < (int)points.length(); i++){
		//calculate goal position
		goalPoint = points[i] * localToWorldMatrix;

		//calculate dampingValue
		dampingMagnitude = damping * dampingMap[i];
		//calculate stiffnessValue
		stiffnessMagnitude = stiffness * stiffnessMap[i];
		//velocity
		velocity = (currentPointsPos[i] - previousPointsPos[i])*(1 - dampingMagnitude);
		newPoint = currentPointsPos[i] + velocity;
		goalForce = (goalPoint - newPoint) *stiffnessMagnitude;
		newPoint += goalForce;

		//setup maximum displacement
		displacement = newPoint - goalPoint;
		if (displacement.length() > maxdiplacement){
			displacement = displacement.normal() * maxdiplacement;
			newPoint = goalPoint + displacement;
		}
		
		//setup jigglepoint direction
		/*if biasdirection is larger than zero, jiggle points could not enter inside the goal point
		else jiggling point could not leave outside the goal point
		*/
		if (biasdirection > 0.0){
			//jiggling points is inside the goal point
			if (displacement * normals[membershipMap[i]] < 0.0f){
				newPoint += normals[membershipMap[i]] * (displacement * normals[membershipMap[i]]) * biasdirection;
			}
		}
		else if (biasdirection < 0.0) {
			if (displacement * normals[membershipMap[i]] > 0.0f){
				newPoint += normals[membershipMap[i]] * (displacement * normals[membershipMap[i]]) * biasdirection;
			}
		}

		previousPointsPos[i] = currentPointsPos[i];
		currentPointsPos[i] = newPoint;

		points[i] += (newPoint * worldtoLocalMatrix - points[i]) * jiggleMap[i] * weights[i] * env;
	}

	ItGeom.setAllPositions(points);
	previousTime = currentTime;
	return status;
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
	nAttr.setKeyable(true);
	addAttribute(mDamping);
	attributeAffects(mDamping, outputGeom);
	
	mMaxDisplacement = nAttr.create("maxDispacement", "maxDisplacement", MFnNumericData::kFloat, 1.0, &status);
	nAttr.setMin(0.0);
	nAttr.setKeyable(true);
	addAttribute(mMaxDisplacement);
	attributeAffects(mMaxDisplacement,outputGeom);

	mScale = nAttr.create("scale", "scale", MFnNumericData::kFloat, 1.0, &status);
	nAttr.setMin(0.0);
	nAttr.setKeyable(true);
	addAttribute(mScale);
	attributeAffects(mScale, outputGeom);

	mBiasDirection = nAttr.create("biasDirection", "biasDirection", MFnNumericData::kFloat, 0.0, &status);
	nAttr.setMin(0.0);
	nAttr.setMax(1.0);
	addAttribute(mBiasDirection);
	attributeAffects(mBiasDirection, outputGeom);

	//compoundAttribute
	mWorldMatrix = mAttr.create("worldMatrix","worldMatrix");

	//jiggleMap
	mJiggleMap = nAttr.create("jiggleMap", "jiggleMap", MFnNumericData::kFloat, 1.0, &status);
	nAttr.setMin(0.0);
	nAttr.setMax(1.0);
	//Sets this attibute should have an array of data.
	//Sets the data for this array from the data in the builder object.
	nAttr.setArray(true);
	//make this attribute changeble
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
	//mPerGeometry
	mPerGeometry = cAttr.create("perGeometry", "perGeometry", &status);
	cAttr.setArray(true);
	cAttr.addChild(mJiggleMap);
	cAttr.addChild(mDampingMap);
	cAttr.addChild(mStiffnessMap);
	cAttr.addChild(mWorldMatrix);
	cAttr.setUsesArrayDataBuilder(true);
	addAttribute(mPerGeometry);

	attributeAffects(mJiggleMap,outputGeom);
	attributeAffects(mStiffnessMap, outputGeom);
	attributeAffects(mDampingMap, outputGeom);
	attributeAffects(mWorldMatrix, outputGeom);

	//make these attributes paintable
	MGlobal::executeCommand("makePaintable -attrType multiFloat -sm deformer jiggleDeformer weights");
	MGlobal::executeCommand("makePaintable -attrType multiFloat -sm deformer jiggleDeformer stiffnessMap");
	MGlobal::executeCommand("makePaintable -attrType multiFloat -sm deformer jiggleDeformer dampingMap");
	MGlobal::executeCommand("makePaintable -attrType multiFloat -sm deformer jiggleDeformer jiggleMap");

	return MS::kSuccess;
}







