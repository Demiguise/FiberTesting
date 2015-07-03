#include "stdafx.h"
#include "Math.h"

//bool HyperplaneSeperationTest(AABB a, AABB b)
//{
//	TransformComponent* pCompA = g_pTransformMgr->GetComponent(idA);
//	TransformComponent* pCompB = g_pTransformMgr->GetComponent(idB);
//
//	//Now for the actual testing.
//	//Box A's Axes
//	return (TestForOverlap(pCompA, pCompB, a, b, pCompA->GetLocalAxis(0)) &&
//		TestForOverlap(pCompA, pCompB, a, b, pCompA->GetLocalAxis(1)) &&
//		TestForOverlap(pCompA, pCompB, a, b, pCompA->GetLocalAxis(2)) &&
//		//Box b's axes		   
//		TestForOverlap(pCompA, pCompB, a, b, pCompB->GetLocalAxis(0)) &&
//		TestForOverlap(pCompA, pCompB, a, b, pCompB->GetLocalAxis(1)) &&
//		TestForOverlap(pCompA, pCompB, a, b, pCompB->GetLocalAxis(2)) &&
//		//Cross products of each axes
//		TestForOverlap(pCompA, pCompB, a, b, pCompA->GetLocalAxis(0).Cross(pCompB->GetLocalAxis(0))) &&
//		TestForOverlap(pCompA, pCompB, a, b, pCompA->GetLocalAxis(0).Cross(pCompB->GetLocalAxis(1))) &&
//		TestForOverlap(pCompA, pCompB, a, b, pCompA->GetLocalAxis(0).Cross(pCompB->GetLocalAxis(2))) &&
//		TestForOverlap(pCompA, pCompB, a, b, pCompA->GetLocalAxis(1).Cross(pCompB->GetLocalAxis(0))) &&
//		TestForOverlap(pCompA, pCompB, a, b, pCompA->GetLocalAxis(1).Cross(pCompB->GetLocalAxis(1))) &&
//		TestForOverlap(pCompA, pCompB, a, b, pCompA->GetLocalAxis(1).Cross(pCompB->GetLocalAxis(2))) &&
//		TestForOverlap(pCompA, pCompB, a, b, pCompA->GetLocalAxis(2).Cross(pCompB->GetLocalAxis(0))) &&
//		TestForOverlap(pCompA, pCompB, a, b, pCompA->GetLocalAxis(2).Cross(pCompB->GetLocalAxis(1))) &&
//		TestForOverlap(pCompA, pCompB, a, b, pCompA->GetLocalAxis(2).Cross(pCompB->GetLocalAxis(2))));
//}
//
//bool TestForOverlap(	const TransformComponent* a,
//																		const TransformComponent* b,
//																		const PhysicsComponent* aPhys,
//																		const PhysicsComponent* bPhys,
//																		const EnVector3& axis)
//{
//	float aProjection = ProjectToAxis(a, aPhys, axis);
//	float bProjection = ProjectToAxis(b, bPhys, axis);
//	EnVector3 abVector = b->GetLocalAxis(3) - a->GetLocalAxis(3);
//	float abVectorProjection = abs(abVector.ADot(axis));
//	return abVectorProjection <= (aProjection + bProjection);
//}
//
//// Just pass Matrix as well
//float PhysicsManager::ProjectToAxis(	const TransformComponent* a,
//																		const PhysicsComponent* aPhys,
//																		const EnVector3& axis)
//{
//	return (abs(aPhys->m_rigidBody.m_halfExtents.x * axis.ADot(a->GetLocalAxis(0))) +
//		abs(aPhys->m_rigidBody.m_halfExtents.y * axis.ADot(a->GetLocalAxis(1))) +
//		abs(aPhys->m_rigidBody.m_halfExtents.z * axis.ADot(a->GetLocalAxis(2))));
//}