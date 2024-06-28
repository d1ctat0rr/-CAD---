#ifndef MINECLEARANCE_H
#define MINECLEARANCE_H
#pragma once

#include <chrono>

class CMineClearance : public AcDbEntity
{


public:
	ACRX_DECLARE_MEMBERS(CMineClearance);

protected:
	static Adesk::UInt32 kCurrentVersionNumber;

public:
	CMineClearance();
	virtual ~CMineClearance();
	virtual Acad::ErrorStatus init();

	//ui代表整个扫雷矩阵，status储存每个格子状态
	int ui[12][12] , status[12][12] ;
	int mineNum = 18;
	int gameStatus = 1;
	bool markStatus = 0;
	int result = 0;

	//----- AcDbObject protocols
	//---- Dwg Filing protocol
	virtual Acad::ErrorStatus dwgOutFields (AcDbDwgFiler *pFiler) const;
	virtual Acad::ErrorStatus dwgInFields (AcDbDwgFiler *pFiler);

	//----- AcDbEntity protocols
	//----- Graphics protocol
protected:
	virtual Adesk::Boolean subWorldDraw (AcGiWorldDraw *mode);
	virtual Adesk::UInt32 subSetAttributes (AcGiDrawableTraits *traits);

public:
	//----- Osnap points protocol
	virtual Acad::ErrorStatus subGetOsnapPoints(
		AcDb::OsnapMode osnapMode,
		Adesk::GsMarker gsSelectionMark,
		const AcGePoint3d &pickPoint,
		const AcGePoint3d &lastPoint,
		const AcGeMatrix3d &viewXform,
		AcGePoint3dArray &snapPoints,
		AcDbIntArray &geomIds) const;

	virtual Acad::ErrorStatus subGetOsnapPoints(
		AcDb::OsnapMode osnapMode,
		Adesk::GsMarker gsSelectionMark,
		const AcGePoint3d &pickPoint,
		const AcGePoint3d &lastPoint,
		const AcGeMatrix3d &viewXform,
		AcGePoint3dArray &snapPoints,
		AcDbIntArray &geomIds,
		const AcGeMatrix3d &insertionMat) const;

	//----- Grip points protocol
	virtual Acad::ErrorStatus subGetGripPoints(AcGePoint3dArray &gripPoints, AcDbIntArray &osnapModes, AcDbIntArray &geomIds) const;
	virtual Acad::ErrorStatus subMoveGripPointsAt(const AcDbIntArray &indices, const AcGeVector3d &offset);
	virtual Acad::ErrorStatus subGetGripPoints(
		AcDbGripDataPtrArray &grips, const double curViewUnitSize, const int gripSize,
		const AcGeVector3d &curViewDir, const int bitflags) const;
	virtual Acad::ErrorStatus subMoveGripPointsAt(const AcDbVoidPtrArray &gripAppData, const AcGeVector3d &offset, const int bitflags);
};

#endif
