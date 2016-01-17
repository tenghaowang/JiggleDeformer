#include <maya/MFnData.h>
#include <maya/MPxDeformerNode.h>
#include <maya/MPxNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnMesh.h>
#include <maya/MFnData.h>
#include <maya/MItGeometry.h>
#include <maya/MTime.h>
#include <maya/MFloatArray.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MMatrix.h>
#include <maya/MGlobal.h>
#include <maya/MTime.h>
#include <map>
#include <maya/MArrayDataBuilder.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MDataHandle.h>
#include <maya/MDataBlock.h>
#include <maya/MFloatVectorArray.h>
#include <string>
#include <maya/MIOStream.h>
#include <omp.h>

class jiggleDeformer : public MPxDeformerNode{
	public :
		jiggleDeformer();
		virtual ~jiggleDeformer();
		//deform function
		virtual MStatus deform(MDataBlock& pDataBlock,
							   MItGeometry& ItGeom,
							   const MMatrix& localToWorldMatrix,
							   unsigned int geomIndex);
		
		virtual MStatus setDependentsDirty(const MPlug& plug, MPlugArray& plugArray);
		
		static void *nodeCreator();
		static MStatus nodeInitialize();
		static MTypeId nodeID;
		static MObject mTime;
		static MObject mDamping;
		static MObject mStiffness;
		static MObject mMaxDisplacement;
		static MObject mScale;
		static MObject mJiggleMap;
		static MObject mDampingMap;
		//this attributes used to lock jiggling directions
		static MObject mNormalStrength;
		static MObject mBiasDirection;
		static MObject mStartFrame;
		static MObject mStiffnessMap;
		static MObject mPerGeometry;
		static MObject mWorldMatrix;

	private:
		MStatus getInputMesh(MDataBlock pDataBlock, unsigned int geomIndex, MObject* mInputMesh);
		MStatus jumpToElement(MArrayDataHandle& dataArray, unsigned int index);
		std::map<unsigned int, MFloatArray> _jiggleMap;
		std::map<unsigned int, MFloatArray> _dampingMap;
		std::map<unsigned int, MFloatArray> _stiffnessMap;
		std::map<unsigned int, MTime> _PreviousTime;
		std::map<unsigned int, bool> _initialized;
		std::map<unsigned int, MPointArray> _previousPointsPos;
		std::map<unsigned int, MPointArray> _currentPointPos;
		std::map<unsigned int, MFloatArray> _weights;
		std::map<unsigned int, MIntArray> _membership;
		std::map<unsigned int, bool>_dirtyMap;



};