#include "JiggleDeformer.h"
#include <maya/MFnPlugin.h>


MStatus initializePlugin(MObject mobject){
	MStatus status;
	MFnPlugin mfnPlugin(mobject, "Ryan Wang", "1.0","Any");
	status = mfnPlugin.registerNode("jiggelDeformer", jiggleDeformer::nodeID, jiggleDeformer::nodeCreator,
											jiggleDeformer::nodeInitialize,MPxNode::kDeformerNode);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}

MStatus uninitializePlugin(MObject mobject){
	MStatus status;
	MFnPlugin mfnPlugin(mobject);
	status = mfnPlugin.deregisterNode(jiggleDeformer::nodeID);
	
	CHECK_MSTATUS_AND_RETURN_IT(status);
	
	return status;
}