#include "pch.h"
#include "MineClearance.h"
#include"dbproxy.h"
#include "dbhatch.h"



//----------------------------------------------------------------------------
Adesk::UInt32 CMineClearance::kCurrentVersionNumber = 1 ;

//----------------------------------------------------------------------------
//---- runtime definition
ACRX_DXF_DEFINE_MEMBERS (
	CMineClearance, AcDbEntity,
	AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent, 
	AcDbProxyEntity::kNoOperation, MINECLEARANCE, MINECLEARANCEAPP
)

//----------------------------------------------------------------------------
//---- construct & destruct

CMineClearance::CMineClearance(){

	init();
}

CMineClearance::~CMineClearance(){}

Acad::ErrorStatus CMineClearance::init() {

	gameStatus = 1;
	markStatus = 0;
	result = 0;

	for (int i = 0; i < 12; i++) {
		for (int j = 0; j < 12; j++) {
			ui[i][j] = 0;
			status[i][j] = 0;
		}
	}


	// ���������
	srand((unsigned)time(0));
	int placedMines = 0;
	while (placedMines < mineNum) {
		int x = rand() % 12;
		int y = rand() % 12;
		if (ui[x][y] == 0) {
			ui[x][y] = -1; // -1��ʾ��
			placedMines++;
		}
	}

	// ����ÿ��������Χ������
	for (int i = 0; i < 12; ++i) {
		for (int j = 0; j < 12; ++j) {
			if (ui[i][j] == -1) continue;
			int minesCount = 0;
			for (int dx = -1; dx <= 1; ++dx) {
				for (int dy = -1; dy <= 1; ++dy) {
					int nx = i + dx;
					int ny = j + dy;
					if (nx >= 0 && nx < 12 && ny >= 0 && ny < 12 && ui[nx][ny] == -1) {
						minesCount++;
					}
				}
			}
			ui[i][j] = minesCount;
		}
	}

	return Acad::eOk;
}

//----------------------------------------------------------------------------
//----- AcDbObject protocols
//---- Dwg Filing protocol
Acad::ErrorStatus CMineClearance::dwgOutFields (AcDbDwgFiler *pFiler) const {
	assertReadEnabled ();
	Acad::ErrorStatus es = AcDbEntity::dwgOutFields(pFiler);
	if (es != Acad::eOk)
		return (es);
	if ((es = pFiler->writeUInt32(CMineClearance::kCurrentVersionNumber)) != Acad::eOk)
		return (es);
	//----- Output params
	//.....

	return (pFiler->filerStatus());
}

Acad::ErrorStatus CMineClearance::dwgInFields(AcDbDwgFiler * pFiler) {
	assertWriteEnabled();
	Acad::ErrorStatus es = AcDbEntity::dwgInFields(pFiler);
	if (es != Acad::eOk)
		return (es);
	Adesk::UInt32 version = 0;
	if ((es = pFiler->readUInt32(&version)) != Acad::eOk)
		return (es);
	if (version > CMineClearance::kCurrentVersionNumber)
		return (Acad::eMakeMeProxy);
	//if ( version < CMineClearance::kCurrentVersionNumber )
	//	return (Acad::eMakeMeProxy) ;
	//----- Read params
	//.....

	return (pFiler->filerStatus());
}

//----------------------------------------------------------------------------
//----- AcDbEntity protocols
Adesk::Boolean CMineClearance::subWorldDraw(AcGiWorldDraw * mode) {
	assertReadEnabled();


	// ���ƾ������
	AcGePoint3dArray points;
	points.append(AcGePoint3d(0, 0, 0));
	points.append(AcGePoint3d(12, 0, 0));
	points.append(AcGePoint3d(12, 12, 0));
	points.append(AcGePoint3d(0, 12, 0));
	points.append(AcGePoint3d(0, 0, 0));
	// ������ɫ
	mode->subEntityTraits().setColor(7); // ����Ϊ��ɫ����ɫ����Ϊ7
	mode->geometry().polyline(5, points.asArrayPtr());



	/*������ڶ����л����״̬��ť*/
	AcGePoint3dArray markButton;
	int mbx = 15;
	int mby = 11;

	markButton.append(AcGePoint3d(mbx, mby, 0));
	markButton.append(AcGePoint3d(mbx+4, mby, 0));
	markButton.append(AcGePoint3d(mbx+4, mby+1, 0));
	markButton.append(AcGePoint3d(mbx, mby+1, 0));
	markButton.append(AcGePoint3d(mbx, mby, 0));
	mode->geometry().polyline(5, markButton.asArrayPtr());

	mode->subEntityTraits().setColor(8); // ����Ϊ��ɫ����ɫ����Ϊ8

	AcGePoint2dArray fillPoints;
	/*fillPoints.append(AcGePoint2d(i, j));
	fillPoints.append(AcGePoint2d(i + 1, j));
	fillPoints.append(AcGePoint2d(i + 1, j + 1));
	fillPoints.append(AcGePoint2d(i, j + 1));
	fillPoints.append(AcGePoint2d(i, j));*/

	fillPoints.append(AcGePoint2d(mbx + 0.1, mby + 0.1));
	fillPoints.append(AcGePoint2d(mbx + 3.9, mby + 0.1));
	fillPoints.append(AcGePoint2d(mbx + 3.9, mby + 0.9));
	fillPoints.append(AcGePoint2d(mbx + 0.1, mby + 0.9));
	fillPoints.append(AcGePoint2d(mbx + 0.1, mby + 0.1));

	AcGeDoubleArray vertexBulges;
	vertexBulges.setPhysicalLength(0).setLogicalLength(5);
	for (int i = 0; i < 5; i++)
		vertexBulges[i] = 0.0;


	AcDbHatch* pHatch = new AcDbHatch();
	AcGeVector3d normal(0.0, 0.0, 1.0); // �������ƽ��ķ�����
	pHatch->setNormal(normal);
	pHatch->setElevation(0.0); // �������ƽ��ĸ߶ȣ��뵱ǰZ������أ�

	pHatch->setAssociative(false); // �ǹ������
	pHatch->setPattern(AcDbHatch::kPreDefined, TEXT("SOLID")); // ʹ��Ԥ�����ʵ�����
	pHatch->setHatchStyle(AcDbHatch::kNormal); // ���������ʽΪ��ͨ

	// ����������

	pHatch->appendLoop(AcDbHatch::kExternal, fillPoints, vertexBulges);

	// ���������
	pHatch->evaluateHatch();

	pHatch->worldDraw(mode);

	delete pHatch;

	mode->subEntityTraits().setColor(250); // ����Ϊ��ɫ����ɫ����Ϊ7

	if (markStatus == 0) {

		std::wstring text = L"��ǵ���";

		// �����ı���С��λ��
		double textHeight = 0.6; // �����ı��ĸ߶ȣ�������Ҫ���е���
		AcGePoint3d textPosition(mbx + 0.2, mby + 0.2, 0); // �ı���ʼ��λ��	

		mode->subEntityTraits().setColor(7);
		mode->geometry().text(
			textPosition,                       // �ı���ʼ��λ��
			AcGeVector3d::kZAxis,               // �ı�ƽ��ķ��߷���
			AcGeVector3d::kXAxis,               // �ı�����
			textHeight,                         // �ı��߶�
			0.0,                                // �ı���ȣ�Ĭ��Ϊ0����ʾʹ�ñ�׼��ȣ�
			0.0,                                // �ı�б��Ƕȣ�Ĭ��Ϊ0����ʾ��б�壩
			text.c_str()                      // Ҫ��ʾ���ı�����
		);


	}

	else if (markStatus == 1) {

		std::wstring text = L"ֹͣ���";

		// �����ı���С��λ��
		double textHeight = 0.6; // �����ı��ĸ߶ȣ�������Ҫ���е���
		AcGePoint3d textPosition(mbx + 0.2, mby + 0.2, 0); // �ı���ʼ��λ��	

		mode->subEntityTraits().setColor(7);
		mode->geometry().text(
			textPosition,                       // �ı���ʼ��λ��
			AcGeVector3d::kZAxis,               // �ı�ƽ��ķ��߷���
			AcGeVector3d::kXAxis,               // �ı�����
			textHeight,                         // �ı��߶�
			0.0,                                // �ı���ȣ�Ĭ��Ϊ0����ʾʹ�ñ�׼��ȣ�
			0.0,                                // �ı�б��Ƕȣ�Ĭ��Ϊ0����ʾ��б�壩
			text.c_str()                      // Ҫ��ʾ���ı�����
		);

	}
	/*������ڶ����л����״̬��ť*/


	/*������ڶ������¿�ʼ��ť*/
	AcGePoint3dArray remakeButton;
	int rbx = 15;
	int rby = 8;

	remakeButton.append(AcGePoint3d(rbx, rby, 0));
	remakeButton.append(AcGePoint3d(rbx + 4, rby, 0));
	remakeButton.append(AcGePoint3d(rbx + 4, rby + 1, 0));
	remakeButton.append(AcGePoint3d(rbx, rby + 1, 0));
	remakeButton.append(AcGePoint3d(rbx, rby, 0));
	mode->geometry().polyline(5, remakeButton.asArrayPtr());

	mode->subEntityTraits().setColor(8); // ����Ϊ��ɫ����ɫ����Ϊ8

	AcGePoint2dArray fillPoints1;
	/*fillPoints.append(AcGePoint2d(i, j));
	fillPoints.append(AcGePoint2d(i + 1, j));
	fillPoints.append(AcGePoint2d(i + 1, j + 1));
	fillPoints.append(AcGePoint2d(i, j + 1));
	fillPoints.append(AcGePoint2d(i, j));*/

	fillPoints1.append(AcGePoint2d(rbx + 0.1, rby + 0.1));
	fillPoints1.append(AcGePoint2d(rbx + 3.9, rby + 0.1));
	fillPoints1.append(AcGePoint2d(rbx + 3.9, rby + 0.9));
	fillPoints1.append(AcGePoint2d(rbx + 0.1, rby + 0.9));
	fillPoints1.append(AcGePoint2d(rbx + 0.1, rby + 0.1));

	AcGeDoubleArray vertexBulges1;
	vertexBulges1.setPhysicalLength(0).setLogicalLength(5);
	for (int i = 0; i < 5; i++)
		vertexBulges1[i] = 0.0;


	AcDbHatch* pHatch1 = new AcDbHatch();
	AcGeVector3d normal1(0.0, 0.0, 1.0); // �������ƽ��ķ�����
	pHatch1->setNormal(normal1);
	pHatch1->setElevation(0.0); // �������ƽ��ĸ߶ȣ��뵱ǰZ������أ�

	pHatch1->setAssociative(false); // �ǹ������
	pHatch1->setPattern(AcDbHatch::kPreDefined, TEXT("SOLID")); // ʹ��Ԥ�����ʵ�����
	pHatch1->setHatchStyle(AcDbHatch::kNormal); // ���������ʽΪ��ͨ

	// ����������

	pHatch1->appendLoop(AcDbHatch::kExternal, fillPoints1, vertexBulges1);

	// ���������
	pHatch1->evaluateHatch();

	pHatch1->worldDraw(mode);

	delete pHatch1;

	mode->subEntityTraits().setColor(250); // ����Ϊ��ɫ����ɫ����Ϊ7



		std::wstring text1 = L"���¿�ʼ";

		// �����ı���С��λ��
		double textHeight1 = 0.6; // �����ı��ĸ߶ȣ�������Ҫ���е���
		AcGePoint3d textPosition1(rbx + 0.2, rby + 0.2, 0); // �ı���ʼ��λ��	

		mode->subEntityTraits().setColor(7);
		mode->geometry().text(
			textPosition1,                       // �ı���ʼ��λ��
			AcGeVector3d::kZAxis,               // �ı�ƽ��ķ��߷���
			AcGeVector3d::kXAxis,               // �ı�����
			textHeight1,                         // �ı��߶�
			0.0,                                // �ı���ȣ�Ĭ��Ϊ0����ʾʹ�ñ�׼��ȣ�
			0.0,                                // �ı�б��Ƕȣ�Ĭ��Ϊ0����ʾ��б�壩
			text1.c_str()                      // Ҫ��ʾ���ı�����
		);

	/*������ڶ������¿�ʼ��ť*/

	/*������ڶ��������Ϸ��ť*/
	AcGePoint3dArray endButton;
	int ebx = 15;
	int eby = 5;

	endButton.append(AcGePoint3d(ebx, eby, 0));
	endButton.append(AcGePoint3d(ebx + 4, eby, 0));
	endButton.append(AcGePoint3d(ebx + 4, eby + 1, 0));
	endButton.append(AcGePoint3d(ebx, eby + 1, 0));
	endButton.append(AcGePoint3d(ebx, eby, 0));
	mode->geometry().polyline(5, endButton.asArrayPtr());

	mode->subEntityTraits().setColor(8); // ����Ϊ��ɫ����ɫ����Ϊ8

	AcGePoint2dArray fillPoints2;
	/*fillPoints.append(AcGePoint2d(i, j));
	fillPoints.append(AcGePoint2d(i + 1, j));
	fillPoints.append(AcGePoint2d(i + 1, j + 1));
	fillPoints.append(AcGePoint2d(i, j + 1));
	fillPoints.append(AcGePoint2d(i, j));*/

	fillPoints2.append(AcGePoint2d(ebx + 0.1, eby + 0.1));
	fillPoints2.append(AcGePoint2d(ebx + 3.9, eby + 0.1));
	fillPoints2.append(AcGePoint2d(ebx + 3.9, eby + 0.9));
	fillPoints2.append(AcGePoint2d(ebx + 0.1, eby + 0.9));
	fillPoints2.append(AcGePoint2d(ebx + 0.1, eby + 0.1));

	AcGeDoubleArray vertexBulges2;
	vertexBulges2.setPhysicalLength(0).setLogicalLength(5);
	for (int i = 0; i < 5; i++)
		vertexBulges2[i] = 0.0;


	AcDbHatch* pHatch2 = new AcDbHatch();
	AcGeVector3d normal2(0.0, 0.0, 1.0); // �������ƽ��ķ�����
	pHatch2->setNormal(normal2);
	pHatch2->setElevation(0.0); // �������ƽ��ĸ߶ȣ��뵱ǰZ������أ�

	pHatch2->setAssociative(false); // �ǹ������
	pHatch2->setPattern(AcDbHatch::kPreDefined, TEXT("SOLID")); // ʹ��Ԥ�����ʵ�����
	pHatch2->setHatchStyle(AcDbHatch::kNormal); // ���������ʽΪ��ͨ

	// ����������

	pHatch2->appendLoop(AcDbHatch::kExternal, fillPoints2, vertexBulges2);

	// ���������
	pHatch2->evaluateHatch();

	pHatch2->worldDraw(mode);

	delete pHatch2;

	mode->subEntityTraits().setColor(250); // ����Ϊ��ɫ����ɫ����Ϊ7


	std::wstring text2 = L"������Ϸ";

	// �����ı���С��λ��
	double textHeight2 = 0.6; // �����ı��ĸ߶ȣ�������Ҫ���е���
	AcGePoint3d textPosition2(ebx + 0.2, eby + 0.2, 0); // �ı���ʼ��λ��	

	mode->subEntityTraits().setColor(7);
	mode->geometry().text(
		textPosition2,                       // �ı���ʼ��λ��
		AcGeVector3d::kZAxis,               // �ı�ƽ��ķ��߷���
		AcGeVector3d::kXAxis,               // �ı�����
		textHeight2,                         // �ı��߶�
		0.0,                                // �ı���ȣ�Ĭ��Ϊ0����ʾʹ�ñ�׼��ȣ�
		0.0,                                // �ı�б��Ƕȣ�Ĭ��Ϊ0����ʾ��б�壩
		text2.c_str()                      // Ҫ��ʾ���ı�����
	);

	/*������ڶ��������ť*/

	mode->subEntityTraits().setColor(7); // ����Ϊ��ɫ����ɫ����Ϊ7

	// ����ÿ������
	for (int i = 0; i < 12; ++i) {
		for (int j = 0; j < 12; ++j) {
			AcGePoint3d lowerLeft(i, j, 0);
			AcGePoint3d lowerRight(i + 1, j, 0);
			AcGePoint3d upperRight(i + 1, j + 1, 0);
			AcGePoint3d upperLeft(i, j + 1, 0);

			// ���Ʒ���߿�
			AcGePoint3dArray boxPoints;
			boxPoints.append(lowerLeft);
			boxPoints.append(lowerRight);
			boxPoints.append(upperRight);
			boxPoints.append(upperLeft);
			boxPoints.append(lowerLeft);
			mode->geometry().polyline(5, boxPoints.asArrayPtr());


			if (status[i][j] == 0) { // δ�㿪��δ��ǵĸ���

				 // ���û�ɫ���
				mode->subEntityTraits().setColor(8); // ����Ϊ��ɫ����ɫ����Ϊ8

				AcGePoint2dArray fillPoints;
				/*fillPoints.append(AcGePoint2d(i, j));
				fillPoints.append(AcGePoint2d(i + 1, j));
				fillPoints.append(AcGePoint2d(i + 1, j + 1));
				fillPoints.append(AcGePoint2d(i, j + 1));
				fillPoints.append(AcGePoint2d(i, j));*/

				fillPoints.append(AcGePoint2d(i + 0.1, j + 0.1));
				fillPoints.append(AcGePoint2d(i + 0.9, j + 0.1));
				fillPoints.append(AcGePoint2d(i + 0.9, j + 0.9));
				fillPoints.append(AcGePoint2d(i + 0.1, j + 0.9));
				fillPoints.append(AcGePoint2d(i + 0.1, j + 0.1));

				AcGeDoubleArray vertexBulges;
				vertexBulges.setPhysicalLength(0).setLogicalLength(5);
				for (int i = 0; i < 5; i++)
					vertexBulges[i] = 0.0;


				AcDbHatch* pHatch = new AcDbHatch();
				AcGeVector3d normal(0.0, 0.0, 1.0); // �������ƽ��ķ�����
				pHatch->setNormal(normal);
				pHatch->setElevation(0.0); // �������ƽ��ĸ߶ȣ��뵱ǰZ������أ�

				pHatch->setAssociative(false); // �ǹ������
				pHatch->setPattern(AcDbHatch::kPreDefined, TEXT("SOLID")); // ʹ��Ԥ�����ʵ�����
				pHatch->setHatchStyle(AcDbHatch::kNormal); // ���������ʽΪ��ͨ

				// ����������

				pHatch->appendLoop(AcDbHatch::kExternal, fillPoints, vertexBulges);

				// ���������
				pHatch->evaluateHatch();

				pHatch->worldDraw(mode);

				delete pHatch;

				mode->subEntityTraits().setColor(7); // ����Ϊ��ɫ����ɫ����Ϊ7

			}
			else if (status[i][j] == 1) { // �ѵ㿪�ĸ���
				if (ui[i][j] == -1) { // �������
					if (1) {
						// ���û�ɫ���
						mode->subEntityTraits().setColor(1); // ����Ϊ��ɫ����ɫ����Ϊ8

						AcGePoint2dArray fillPoints;
						/*fillPoints.append(AcGePoint2d(i, j));
						fillPoints.append(AcGePoint2d(i + 1, j));
						fillPoints.append(AcGePoint2d(i + 1, j + 1));
						fillPoints.append(AcGePoint2d(i, j + 1));
						fillPoints.append(AcGePoint2d(i, j));*/

						fillPoints.append(AcGePoint2d(i + 0.1, j + 0.1));
						fillPoints.append(AcGePoint2d(i + 0.9, j + 0.1));
						fillPoints.append(AcGePoint2d(i + 0.9, j + 0.9));
						fillPoints.append(AcGePoint2d(i + 0.1, j + 0.9));
						fillPoints.append(AcGePoint2d(i + 0.1, j + 0.1));

						AcGeDoubleArray vertexBulges;
						vertexBulges.setPhysicalLength(0).setLogicalLength(5);
						for (int i = 0; i < 5; i++)
							vertexBulges[i] = 0.0;


						AcDbHatch* pHatch = new AcDbHatch();
						AcGeVector3d normal(0.0, 0.0, 1.0); // �������ƽ��ķ�����
						pHatch->setNormal(normal);
						pHatch->setElevation(0.0); // �������ƽ��ĸ߶ȣ��뵱ǰZ������أ�

						pHatch->setAssociative(false); // �ǹ������
						pHatch->setPattern(AcDbHatch::kPreDefined, TEXT("SOLID")); // ʹ��Ԥ�����ʵ�����
						pHatch->setHatchStyle(AcDbHatch::kNormal); // ���������ʽΪ��ͨ

						// ����������

						pHatch->appendLoop(AcDbHatch::kExternal, fillPoints, vertexBulges);

						// ���������
						pHatch->evaluateHatch();

						pHatch->worldDraw(mode);

						delete pHatch;

						mode->subEntityTraits().setColor(7); // ����Ϊ��ɫ����ɫ����Ϊ7
					}


					// ���ú�ɫ���Բ
					mode->subEntityTraits().setColor(250); // ��ɫ����ɫ������1
					//mode->geometry().circle(AcGePoint3d(i + 0.5, j + 0.5, 0), 0.3, AcGeVector3d::kZAxis);

					   // ����һ��Բ
					AcGePoint2d cenPt(i + 0.5, j + 0.5);
					double TWOPI = 2.0 * 3.1415926535897932;
					AcGeCircArc2d *cirArc = new AcGeCircArc2d();
					cirArc->setCenter(cenPt);
					cirArc->setRadius(0.3);
					cirArc->setAngles(0.0, TWOPI);

					AcDbHatch* pHatch = new AcDbHatch();
					AcGeVector3d normal(0.0, 0.0, 1.0); // �������ƽ��ķ�����
					pHatch->setNormal(normal);
					pHatch->setElevation(0.0); // �������ƽ��ĸ߶ȣ��뵱ǰZ������أ�

					pHatch->setAssociative(false); // �ǹ������
					pHatch->setPattern(AcDbHatch::kPreDefined, TEXT("SOLID")); // ʹ��Ԥ�����ʵ�����
					pHatch->setHatchStyle(AcDbHatch::kNormal); // ���������ʽΪ��ͨ

					// ��Բ����ӵ����߽�
					AcGeIntArray edgeTypes;
					AcGeVoidPointerArray edgePtrs;
					edgeTypes.append(AcDbHatch::kCirArc);
					edgePtrs.append((void*)cirArc);
					pHatch->appendLoop(AcDbHatch::kDefault, edgePtrs, edgeTypes);


					// ���������
					pHatch->evaluateHatch();

					pHatch->worldDraw(mode);

					delete pHatch;

					mode->subEntityTraits().setColor(7); // ����Ϊ��ɫ����ɫ����Ϊ7
				}
				else { // ��ʾ��Χ������
					std::wstring number = std::to_wstring(ui[i][j]);

					// �����ı���С��λ��
					double textHeight = 0.6; // �����ı��ĸ߶ȣ�������Ҫ���е���
					AcGePoint3d textPosition(i + 0.2, j + 0.2, 0); // �ı���ʼ��λ��	

					mode->subEntityTraits().setColor(3); // ��ɫ����ɫ����Ϊ3
					mode->geometry().text(
						textPosition,                       // �ı���ʼ��λ��
						AcGeVector3d::kZAxis,               // �ı�ƽ��ķ��߷���
						AcGeVector3d::kXAxis,               // �ı�����
						textHeight,                         // �ı��߶�
						0.0,                                // �ı���ȣ�Ĭ��Ϊ0����ʾʹ�ñ�׼��ȣ�
						0.0,                                // �ı�б��Ƕȣ�Ĭ��Ϊ0����ʾ��б�壩
						number.c_str()                      // Ҫ��ʾ���ı�����
					);

					mode->subEntityTraits().setColor(7); // ����Ϊ��ɫ����ɫ����Ϊ7
				}
			}
			else if (status[i][j] == 2) { // �ѱ�ǵĸ���

				// ���û�ɫ���
				mode->subEntityTraits().setColor(8); // ����Ϊ��ɫ����ɫ����Ϊ8

				AcGePoint2dArray fillPoints;
				/*fillPoints.append(AcGePoint2d(i, j));
				fillPoints.append(AcGePoint2d(i + 1, j));
				fillPoints.append(AcGePoint2d(i + 1, j + 1));
				fillPoints.append(AcGePoint2d(i, j + 1));
				fillPoints.append(AcGePoint2d(i, j));*/

				fillPoints.append(AcGePoint2d(i + 0.1, j + 0.1));
				fillPoints.append(AcGePoint2d(i + 0.9, j + 0.1));
				fillPoints.append(AcGePoint2d(i + 0.9, j + 0.9));
				fillPoints.append(AcGePoint2d(i + 0.1, j + 0.9));
				fillPoints.append(AcGePoint2d(i + 0.1, j + 0.1));

				AcGeDoubleArray vertexBulges;
				vertexBulges.setPhysicalLength(0).setLogicalLength(5);
				for (int i = 0; i < 5; i++)
					vertexBulges[i] = 0.0;


				AcDbHatch* pHatch = new AcDbHatch();
				AcGeVector3d normal(0.0, 0.0, 1.0); // �������ƽ��ķ�����
				pHatch->setNormal(normal);
				pHatch->setElevation(0.0); // �������ƽ��ĸ߶ȣ��뵱ǰZ������أ�

				pHatch->setAssociative(false); // �ǹ������
				pHatch->setPattern(AcDbHatch::kPreDefined, TEXT("SOLID")); // ʹ��Ԥ�����ʵ�����
				pHatch->setHatchStyle(AcDbHatch::kNormal); // ���������ʽΪ��ͨ

				// ����������

				pHatch->appendLoop(AcDbHatch::kExternal, fillPoints, vertexBulges);

				// ���������
				pHatch->evaluateHatch();

				pHatch->worldDraw(mode);

				delete pHatch;

				mode->subEntityTraits().setColor(7); // ����Ϊ��ɫ����ɫ����Ϊ7



				//// ���ú�ɫԲ��
				//mode->subEntityTraits().setColor(1);
				//// ����Բ
				//mode->geometry().circle(AcGePoint3d(i + 0.5, j + 0.5, 0), 0.3, AcGeVector3d::kZAxis);

				if (1) {
					// ���ú�ɫ���Բ
					mode->subEntityTraits().setColor(1); // ��ɫ����ɫ������1
					//mode->geometry().circle(AcGePoint3d(i + 0.5, j + 0.5, 0), 0.3, AcGeVector3d::kZAxis);

					   // ����һ��Բ
					AcGePoint2d cenPt(i + 0.5, j + 0.5);
					double TWOPI = 2.0 * 3.1415926535897932;
					AcGeCircArc2d *cirArc = new AcGeCircArc2d();
					cirArc->setCenter(cenPt);
					cirArc->setRadius(0.3);
					cirArc->setAngles(0.0, TWOPI);

					AcDbHatch* pHatch = new AcDbHatch();
					AcGeVector3d normal(0.0, 0.0, 1.0); // �������ƽ��ķ�����
					pHatch->setNormal(normal);
					pHatch->setElevation(0.0); // �������ƽ��ĸ߶ȣ��뵱ǰZ������أ�

					pHatch->setAssociative(false); // �ǹ������
					pHatch->setPattern(AcDbHatch::kPreDefined, TEXT("SOLID")); // ʹ��Ԥ�����ʵ�����
					pHatch->setHatchStyle(AcDbHatch::kNormal); // ���������ʽΪ��ͨ

					// ��Բ����ӵ����߽�
					AcGeIntArray edgeTypes;
					AcGeVoidPointerArray edgePtrs;
					edgeTypes.append(AcDbHatch::kCirArc);
					edgePtrs.append((void*)cirArc);
					pHatch->appendLoop(AcDbHatch::kDefault, edgePtrs, edgeTypes);


					// ���������
					pHatch->evaluateHatch();

					pHatch->worldDraw(mode);

					delete pHatch;


				}

				mode->subEntityTraits().setColor(7); // ����Ϊ��ɫ����ɫ����Ϊ7
			}
			mode->subEntityTraits().setColor(7);
		}
	}

	
	/*������ڶ�������ʾ����*/
	if (result == 1) {
		std::wstring text = L"����ɨ�״�����������";

		// �����ı���С��λ��
		double textHeight = 2.5; // �����ı��ĸ߶ȣ�������Ҫ���е���
		AcGePoint3d textPosition(-7, 5, 0); // �ı���ʼ��λ��	

		mode->subEntityTraits().setColor(7);
		mode->geometry().text(
			textPosition,                       // �ı���ʼ��λ��
			AcGeVector3d::kZAxis,               // �ı�ƽ��ķ��߷���
			AcGeVector3d::kXAxis,               // �ı�����
			textHeight,                         // �ı��߶�
			0.0,                                // �ı���ȣ�Ĭ��Ϊ0����ʾʹ�ñ�׼��ȣ�
			0.0,                                // �ı�б��Ƕȣ�Ĭ��Ϊ0����ʾ��б�壩
			text.c_str()                      // Ҫ��ʾ���ı�����
		);
	}
	else if (result == 2) {
		std::wstring text = L"�����ˣ�";

		// �����ı���С��λ��
		double textHeight = 4; // �����ı��ĸ߶ȣ�������Ҫ���е���
		AcGePoint3d textPosition(-1, 5, 0); // �ı���ʼ��λ��	

		mode->subEntityTraits().setColor(7);
		mode->geometry().text(
			textPosition,                       // �ı���ʼ��λ��
			AcGeVector3d::kZAxis,               // �ı�ƽ��ķ��߷���
			AcGeVector3d::kXAxis,               // �ı�����
			textHeight,                         // �ı��߶�
			0.0,                                // �ı���ȣ�Ĭ��Ϊ0����ʾʹ�ñ�׼��ȣ�
			0.0,                                // �ı�б��Ƕȣ�Ĭ��Ϊ0����ʾ��б�壩
			text.c_str()                      // Ҫ��ʾ���ı�����
		);
	}


	return (AcDbEntity::subWorldDraw(mode));
}

Adesk::UInt32 CMineClearance::subSetAttributes(AcGiDrawableTraits * traits) {
	assertReadEnabled();
	return (AcDbEntity::subSetAttributes(traits));
}

//----- Osnap points protocol
Acad::ErrorStatus CMineClearance::subGetOsnapPoints(
	AcDb::OsnapMode osnapMode,
	Adesk::GsMarker gsSelectionMark,
	const AcGePoint3d &pickPoint,
	const AcGePoint3d &lastPoint,
	const AcGeMatrix3d &viewXform,
	AcGePoint3dArray &snapPoints,
	AcDbIntArray &geomIds) const{
	assertReadEnabled();

	for (int i = 0; i < 12; ++i) {
		for (int j = 0; j < 12; ++j) {
			// Assuming each cell is 1 unit in size
			AcGePoint3d center(i + 0.5, j + 0.5, 0); // Center point of the cell
			snapPoints.append(center);

			//// Optionally, add snap points for the corners of the cell
			//snapPoints.append(AcGePoint3d(i, j, 0));
			//snapPoints.append(AcGePoint3d(i + 1, j, 0));
			//snapPoints.append(AcGePoint3d(i + 1, j + 1, 0));
			//snapPoints.append(AcGePoint3d(i, j + 1, 0));
		}
	}

	AcGePoint3d markButtonPosition(15 + 2, 11 + 0.5, 0);
	snapPoints.append(markButtonPosition);

	AcGePoint3d remakeButtonPosition(15 + 2, 8 + 0.5, 0);
	snapPoints.append(remakeButtonPosition);

	AcGePoint3d endButtonPosition(15 + 2, 5 + 0.5, 0);
	snapPoints.append(endButtonPosition);

	//return (AcDbEntity::subGetOsnapPoints(osnapMode, gsSelectionMark, pickPoint, lastPoint, viewXform, snapPoints, geomIds));
	return  Acad::eOk;
}

Acad::ErrorStatus CMineClearance::subGetOsnapPoints(
	AcDb::OsnapMode osnapMode,
	Adesk::GsMarker gsSelectionMark,
	const AcGePoint3d &pickPoint,
	const AcGePoint3d &lastPoint,
	const AcGeMatrix3d &viewXform,
	AcGePoint3dArray &snapPoints,
	AcDbIntArray &geomIds,
	const AcGeMatrix3d &insertionMat) const{
	assertReadEnabled();
	return (AcDbEntity::subGetOsnapPoints(osnapMode, gsSelectionMark, pickPoint, lastPoint, viewXform, snapPoints, geomIds, insertionMat));
}

//----- Grip points protocol
Acad::ErrorStatus CMineClearance::subGetGripPoints(
	AcGePoint3dArray &gripPoints, AcDbIntArray &osnapModes, AcDbIntArray &geomIds
) const {
	assertReadEnabled();
	//----- This method is never called unless you return eNotImplemented 
	//----- from the new getGripPoints() method below (which is the default implementation)

	AcGePoint3d vertex1(0, 0, 0);
	AcGePoint3d vertex2(12, 0, 0);
	AcGePoint3d vertex3(12, 12, 0);
	AcGePoint3d vertex4(0, 12, 0);
	gripPoints.append(vertex1);
	gripPoints.append(vertex2);
	gripPoints.append(vertex3);
	gripPoints.append(vertex4);



	/*return (AcDbEntity::subGetGripPoints(gripPoints, osnapModes, geomIds));*/
	return Acad::eOk;
}

Acad::ErrorStatus CMineClearance::subMoveGripPointsAt(const AcDbIntArray &indices, const AcGeVector3d &offset) {
	assertWriteEnabled();
	//----- This method is never called unless you return eNotImplemented 
	//----- from the new moveGripPointsAt() method below (which is the default implementation)

	return (AcDbEntity::subMoveGripPointsAt(indices, offset));
}

Acad::ErrorStatus CMineClearance::subGetGripPoints(
	AcDbGripDataPtrArray &grips, const double curViewUnitSize, const int gripSize,
	const AcGeVector3d &curViewDir, const int bitflags
) const {
	assertReadEnabled();

	//----- If you return eNotImplemented here, that will force AutoCAD to call
	//----- the older getGripPoints() implementation. The call below may return
	//----- eNotImplemented depending of your base class.
	return (AcDbEntity::subGetGripPoints(grips, curViewUnitSize, gripSize, curViewDir, bitflags));
}

Acad::ErrorStatus CMineClearance::subMoveGripPointsAt(const AcDbVoidPtrArray &gripAppData, const AcGeVector3d &offset, 
	const int bitflags
) {
	assertWriteEnabled();

	//----- If you return eNotImplemented here, that will force AutoCAD to call
	//----- the older getGripPoints() implementation. The call below may return
	//----- eNotImplemented depending of your base class.
	return (AcDbEntity::subMoveGripPointsAt(gripAppData, offset, bitflags));
}
