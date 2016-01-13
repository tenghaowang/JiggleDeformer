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

#include <maya/MMatrix.h>
#include <maya/MGlobal.h>
#include <maya/MTime.h>
#include <map>

class jiggleDeformer : public MPxDeformerNode{
	public :
		jiggleDeformer();
		virtual ~jiggleDeformer();
		//deform function
		virtual MStatus deform(MDataBlock& pDataBlock,
							   MItGeometry& ItGeom,
							   const MMatrix& localToWorldMatrix,
							   unsigned int geomIndex);
		
		

		static void *nodeCreator();
		static MStatus nodeInitialize();
		static MTypeId nodeID;
		static MObject mTime;
		static MObject mDamping;
		static MObject mStiffness;
		static MObject mJiggleMap;
		static MObject mDampingMap;
		static MObject mStiffnessMap;

	private:
		std::map<unsigned int, MFloatArray> _jiggleMap;
		std::map<unsigned int, MFloatArray> _dampingMap;
		std::map<unsigned int, MFloatArray> __stiffnessMap;
		std::map<unsigned int, MTime> _PreviousTime;
		std::map<unsigned int, bool> _initialized;
		std::map<unsigned int, MPointArray> _previousPosition;
		std::map<unsigned int, MPointArray> _currentPosition;
		std::map<unsigned int, MFloatArray> _weights;
		std::map<unsigned int, MIntArray> _membership;



};