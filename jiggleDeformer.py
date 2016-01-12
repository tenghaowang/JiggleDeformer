import maya.OpenMaya as OpenMaya
import maya.OpenMayaMPx as OpenMayaMPx
import sys
import math

nodeName = 'jiggleNode'
nodeID = OpenMaya.MTypeId(0x00000720)

class jiggleNode(OpenMayaMPx.MPxNode):
	#static variables
	mOutput = OpenMaya.MObject()
	mGoal = OpenMaya.MObject()
	mDamping = OpenMaya.MObject()
	mStiffness = OpenMaya.MObject()
	mJiggleAmount = OpenMaya.MObject()
	mTime = OpenMaya.MObject()
	parentInverse = OpenMaya.MObject()


	def __init__(self):
		OpenMayaMPx.MPxNode.__init__(self)
		self.initialized = False
		self.currentPosition = OpenMaya.MPoint()
		self.previousPosition = OpenMaya.MPoint()
		self.previousTime =OpenMaya.MTime()

	def compute(self, pPlug, pDataBlock):
		if pPlug == jiggleNode.mOutput:
			dampingHandle = pDataBlock.inputValue(jiggleNode.mDamping)
			dampingValue = dampingHandle.asFloat()

			stiffnessHandle = pDataBlock.inputValue(jiggleNode.mStiffness)
			stiffnessValue = stiffnessHandle.asFloat()

			goalHandle = pDataBlock.inputValue(jiggleNode.mGoal)
			goalPosition = goalHandle.asFloatVector()
			goalPoint = OpenMaya.MPoint(goalPosition)

			timeHandle = pDataBlock.inputValue(jiggleNode.mTime)
			currentTime = timeHandle.asTime()

			parentInverseHandle = pDataBlock.inputValue(jiggleNode.parentInverse)
			parentInverseMatrix = parentInverseHandle.asMatrix()

			jiggleAmountHandle = pDataBlock.inputValue(jiggleNode.mJiggleAmount)
			jiggleAmountValue = jiggleAmountHandle.asFloat()

			if not self.initialized:
				self.previousTime = currentTime
				self.currentPosition = goalPoint
				self.previousPosition = goalPoint
				self.initialized = True

			#-----------------need a stable simulation
			if currentTime.value() -self.previousTime.value() > 1.0 or currentTime.value()< self.previousTime.value():
				self.initialized=False
				self.previousTime = currentTime
				pDataBlock.setClean(pPlug)
				print 'large time gap, simulation terminated!'

			#-----------------algotithm
			velocity = (self.currentPosition - self.previousPosition) * (1-dampingValue)
			newPosition = self.currentPosition + velocity
			goalForce = (goalPoint - newPosition) * stiffnessValue
			newPosition += goalForce

			newPosition = goalPoint + (newPosition-goalPoint) * jiggleAmountValue 
			#-----------------store node status for next computation
			self.previousPosition = OpenMaya.MPoint(self.currentPosition)
			self.currentPosition = OpenMaya.MPoint(newPosition)
			self.previousTime = OpenMaya.MTime(currentTime)
			#convert result to local space
			newPosition *=parentInverseMatrix
			houtput = pDataBlock.outputValue(jiggleNode.mOutput)
			outputVector = OpenMaya.MFloatVector(newPosition.x, newPosition.y, newPosition.z)
			houtput.setMFloatVector(outputVector)
			houtput.setClean()
			pDataBlock.setClean(pPlug)



		else:
			return 'Unknown'


#Creator 
def nodeCreator():
	return  OpenMayaMPx.asMPxPtr(jiggleNode())

#initialize Node Attribute
def nodeInitializer():
	mfnNumericAttr = OpenMaya.MFnNumericAttribute()
	mfnUnitAttr = OpenMaya.MFnUnitAttribute()
	mfnMatrixAttr = OpenMaya.MFnMatrixAttribute()
	#define input
	jiggleNode.mGoal = mfnNumericAttr.createPoint('goal','goal')
	jiggleNode.addAttribute(jiggleNode.mGoal)

	jiggleNode.mDamping = mfnNumericAttr.create('damping', 'damping', OpenMaya.MFnNumericData.kFloat,1.0)
	mfnNumericAttr.setMin(0.0)
	mfnNumericAttr.setMax(1.0)
	mfnNumericAttr.setKeyable(True)
	jiggleNode.addAttribute(jiggleNode.mDamping)

	jiggleNode.mStiffness = mfnNumericAttr.create('stiffness','stiffness',OpenMaya.MFnNumericData.kFloat,1.0)
	mfnNumericAttr.setKeyable(True)
	mfnNumericAttr.setMin(0.0)
	mfnNumericAttr.setMax(1.0)
	jiggleNode.addAttribute(jiggleNode.mStiffness)

	jiggleNode.mJiggleAmount = mfnNumericAttr.create('jiggleAmount', 'jiggleAmount',OpenMaya.MFnNumericData.kFloat,0.0)
	mfnNumericAttr.setKeyable(True)
	mfnNumericAttr.setMin(0.0)
	mfnNumericAttr.setMax(1.0)
	jiggleNode.addAttribute(jiggleNode.mJiggleAmount)

	jiggleNode.mTime = mfnUnitAttr.create('time', 'time', OpenMaya.MFnUnitAttribute.kTime,0.0)
	jiggleNode.addAttribute(jiggleNode.mTime)

	jiggleNode.parentInverse = mfnMatrixAttr.create('parentInverse', 'parentInverse')
	jiggleNode.addAttribute(jiggleNode.parentInverse)
	#define output
	jiggleNode.mOutput = mfnNumericAttr.createPoint('output','output')
	mfnNumericAttr.setWritable(False)
	mfnNumericAttr.setStorable(False)
	jiggleNode.addAttribute(jiggleNode.mOutput)


	#set dirty
	jiggleNode.attributeAffects(jiggleNode.mGoal, jiggleNode.mOutput)
	jiggleNode.attributeAffects(jiggleNode.mStiffness, jiggleNode.mOutput)
	jiggleNode.attributeAffects(jiggleNode.mDamping,jiggleNode.mOutput)
	jiggleNode.attributeAffects(jiggleNode.mJiggleAmount, jiggleNode.mOutput)
	jiggleNode.attributeAffects(jiggleNode.mTime,jiggleNode.mOutput)

#register plugin
def initializePlugin(mobject):
	fnPlugin = OpenMayaMPx.MFnPlugin(mobject, 'Ryan Wang', '1,0', 'Any')
	try:
		fnPlugin.registerNode(nodeName, nodeID, nodeCreator, nodeInitializer)
	except:
		sys.stderr.write('Failed to register node:' + nodeName)
		raise

#deregister plugin
def uninitializePlugin(mobject):
	fnPlugin = OpenMayaMPx.MFnPlugin(mobject)
	try:
		fnPlugin.deregisterNode(nodeName)
	except:
		sys.stderr.write('Failed to deregister node' + nodeName)
		raise
