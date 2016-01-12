#include <maya/MFnData.h>
#include <maya/MPxDeformerNode.h>
#include <maya/MPxNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnMesh.h>
#include <maya/MFnData.h>

#include <maya/MMatrix.h>
#include <maya/MGlobal.h>
#include <maya/MTime.h>

class jiggleDeformer : public MPxDeformerNode{
	public :
		jiggleDeformer();
		virtual ~jiggleDeformer();
		//deform function
		virtual MStatus deform(MDataBlock& data,
							   MItGeometry& iter,
							   const MMatrix& matt,
							    unsigned int mIndex);
		
		
		static void *creator();
		static MStatus initialize();

		static MTypeId nodeID;
		static MObject mTime;
		static MObject mDamping;
		static MObject mStiffness;
		static MObject mJiggleMap;








};