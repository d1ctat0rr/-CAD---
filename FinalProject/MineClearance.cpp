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


	// 随机布置雷
	srand((unsigned)time(0));
	int placedMines = 0;
	while (placedMines < mineNum) {
		int x = rand() % 12;
		int y = rand() % 12;
		if (ui[x][y] == 0) {
			ui[x][y] = -1; // -1表示雷
			placedMines++;
		}
	}

	// 计算每个方块周围的雷数
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


	// 绘制矩阵外框
	AcGePoint3dArray points;
	points.append(AcGePoint3d(0, 0, 0));
	points.append(AcGePoint3d(12, 0, 0));
	points.append(AcGePoint3d(12, 12, 0));
	points.append(AcGePoint3d(0, 12, 0));
	points.append(AcGePoint3d(0, 0, 0));
	// 设置颜色
	mode->subEntityTraits().setColor(7); // 设置为白色，颜色索引为7
	mode->geometry().polyline(5, points.asArrayPtr());



	/*这段是在定义切换标记状态按钮*/
	AcGePoint3dArray markButton;
	int mbx = 15;
	int mby = 11;

	markButton.append(AcGePoint3d(mbx, mby, 0));
	markButton.append(AcGePoint3d(mbx+4, mby, 0));
	markButton.append(AcGePoint3d(mbx+4, mby+1, 0));
	markButton.append(AcGePoint3d(mbx, mby+1, 0));
	markButton.append(AcGePoint3d(mbx, mby, 0));
	mode->geometry().polyline(5, markButton.asArrayPtr());

	mode->subEntityTraits().setColor(8); // 设置为灰色，颜色索引为8

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
	AcGeVector3d normal(0.0, 0.0, 1.0); // 设置填充平面的法向量
	pHatch->setNormal(normal);
	pHatch->setElevation(0.0); // 设置填充平面的高度（与当前Z坐标相关）

	pHatch->setAssociative(false); // 非关联填充
	pHatch->setPattern(AcDbHatch::kPreDefined, TEXT("SOLID")); // 使用预定义的实体填充
	pHatch->setHatchStyle(AcDbHatch::kNormal); // 设置填充样式为普通

	// 添加填充区域

	pHatch->appendLoop(AcDbHatch::kExternal, fillPoints, vertexBulges);

	// 完成填充计算
	pHatch->evaluateHatch();

	pHatch->worldDraw(mode);

	delete pHatch;

	mode->subEntityTraits().setColor(250); // 设置为白色，颜色索引为7

	if (markStatus == 0) {

		std::wstring text = L"标记地雷";

		// 调整文本大小和位置
		double textHeight = 0.6; // 调整文本的高度，根据需要进行调整
		AcGePoint3d textPosition(mbx + 0.2, mby + 0.2, 0); // 文本起始点位置	

		mode->subEntityTraits().setColor(7);
		mode->geometry().text(
			textPosition,                       // 文本起始点位置
			AcGeVector3d::kZAxis,               // 文本平面的法线方向
			AcGeVector3d::kXAxis,               // 文本方向
			textHeight,                         // 文本高度
			0.0,                                // 文本宽度（默认为0，表示使用标准宽度）
			0.0,                                // 文本斜体角度（默认为0，表示无斜体）
			text.c_str()                      // 要显示的文本内容
		);


	}

	else if (markStatus == 1) {

		std::wstring text = L"停止标记";

		// 调整文本大小和位置
		double textHeight = 0.6; // 调整文本的高度，根据需要进行调整
		AcGePoint3d textPosition(mbx + 0.2, mby + 0.2, 0); // 文本起始点位置	

		mode->subEntityTraits().setColor(7);
		mode->geometry().text(
			textPosition,                       // 文本起始点位置
			AcGeVector3d::kZAxis,               // 文本平面的法线方向
			AcGeVector3d::kXAxis,               // 文本方向
			textHeight,                         // 文本高度
			0.0,                                // 文本宽度（默认为0，表示使用标准宽度）
			0.0,                                // 文本斜体角度（默认为0，表示无斜体）
			text.c_str()                      // 要显示的文本内容
		);

	}
	/*这段是在定义切换标记状态按钮*/


	/*这段是在定义重新开始按钮*/
	AcGePoint3dArray remakeButton;
	int rbx = 15;
	int rby = 8;

	remakeButton.append(AcGePoint3d(rbx, rby, 0));
	remakeButton.append(AcGePoint3d(rbx + 4, rby, 0));
	remakeButton.append(AcGePoint3d(rbx + 4, rby + 1, 0));
	remakeButton.append(AcGePoint3d(rbx, rby + 1, 0));
	remakeButton.append(AcGePoint3d(rbx, rby, 0));
	mode->geometry().polyline(5, remakeButton.asArrayPtr());

	mode->subEntityTraits().setColor(8); // 设置为灰色，颜色索引为8

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
	AcGeVector3d normal1(0.0, 0.0, 1.0); // 设置填充平面的法向量
	pHatch1->setNormal(normal1);
	pHatch1->setElevation(0.0); // 设置填充平面的高度（与当前Z坐标相关）

	pHatch1->setAssociative(false); // 非关联填充
	pHatch1->setPattern(AcDbHatch::kPreDefined, TEXT("SOLID")); // 使用预定义的实体填充
	pHatch1->setHatchStyle(AcDbHatch::kNormal); // 设置填充样式为普通

	// 添加填充区域

	pHatch1->appendLoop(AcDbHatch::kExternal, fillPoints1, vertexBulges1);

	// 完成填充计算
	pHatch1->evaluateHatch();

	pHatch1->worldDraw(mode);

	delete pHatch1;

	mode->subEntityTraits().setColor(250); // 设置为白色，颜色索引为7



		std::wstring text1 = L"重新开始";

		// 调整文本大小和位置
		double textHeight1 = 0.6; // 调整文本的高度，根据需要进行调整
		AcGePoint3d textPosition1(rbx + 0.2, rby + 0.2, 0); // 文本起始点位置	

		mode->subEntityTraits().setColor(7);
		mode->geometry().text(
			textPosition1,                       // 文本起始点位置
			AcGeVector3d::kZAxis,               // 文本平面的法线方向
			AcGeVector3d::kXAxis,               // 文本方向
			textHeight1,                         // 文本高度
			0.0,                                // 文本宽度（默认为0，表示使用标准宽度）
			0.0,                                // 文本斜体角度（默认为0，表示无斜体）
			text1.c_str()                      // 要显示的文本内容
		);

	/*这段是在定义重新开始按钮*/

	/*这段是在定义结束游戏按钮*/
	AcGePoint3dArray endButton;
	int ebx = 15;
	int eby = 5;

	endButton.append(AcGePoint3d(ebx, eby, 0));
	endButton.append(AcGePoint3d(ebx + 4, eby, 0));
	endButton.append(AcGePoint3d(ebx + 4, eby + 1, 0));
	endButton.append(AcGePoint3d(ebx, eby + 1, 0));
	endButton.append(AcGePoint3d(ebx, eby, 0));
	mode->geometry().polyline(5, endButton.asArrayPtr());

	mode->subEntityTraits().setColor(8); // 设置为灰色，颜色索引为8

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
	AcGeVector3d normal2(0.0, 0.0, 1.0); // 设置填充平面的法向量
	pHatch2->setNormal(normal2);
	pHatch2->setElevation(0.0); // 设置填充平面的高度（与当前Z坐标相关）

	pHatch2->setAssociative(false); // 非关联填充
	pHatch2->setPattern(AcDbHatch::kPreDefined, TEXT("SOLID")); // 使用预定义的实体填充
	pHatch2->setHatchStyle(AcDbHatch::kNormal); // 设置填充样式为普通

	// 添加填充区域

	pHatch2->appendLoop(AcDbHatch::kExternal, fillPoints2, vertexBulges2);

	// 完成填充计算
	pHatch2->evaluateHatch();

	pHatch2->worldDraw(mode);

	delete pHatch2;

	mode->subEntityTraits().setColor(250); // 设置为白色，颜色索引为7


	std::wstring text2 = L"结束游戏";

	// 调整文本大小和位置
	double textHeight2 = 0.6; // 调整文本的高度，根据需要进行调整
	AcGePoint3d textPosition2(ebx + 0.2, eby + 0.2, 0); // 文本起始点位置	

	mode->subEntityTraits().setColor(7);
	mode->geometry().text(
		textPosition2,                       // 文本起始点位置
		AcGeVector3d::kZAxis,               // 文本平面的法线方向
		AcGeVector3d::kXAxis,               // 文本方向
		textHeight2,                         // 文本高度
		0.0,                                // 文本宽度（默认为0，表示使用标准宽度）
		0.0,                                // 文本斜体角度（默认为0，表示无斜体）
		text2.c_str()                      // 要显示的文本内容
	);

	/*这段是在定义结束按钮*/

	mode->subEntityTraits().setColor(7); // 设置为白色，颜色索引为7

	// 绘制每个方块
	for (int i = 0; i < 12; ++i) {
		for (int j = 0; j < 12; ++j) {
			AcGePoint3d lowerLeft(i, j, 0);
			AcGePoint3d lowerRight(i + 1, j, 0);
			AcGePoint3d upperRight(i + 1, j + 1, 0);
			AcGePoint3d upperLeft(i, j + 1, 0);

			// 绘制方块边框
			AcGePoint3dArray boxPoints;
			boxPoints.append(lowerLeft);
			boxPoints.append(lowerRight);
			boxPoints.append(upperRight);
			boxPoints.append(upperLeft);
			boxPoints.append(lowerLeft);
			mode->geometry().polyline(5, boxPoints.asArrayPtr());


			if (status[i][j] == 0) { // 未点开且未标记的格子

				 // 设置灰色填充
				mode->subEntityTraits().setColor(8); // 设置为灰色，颜色索引为8

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
				AcGeVector3d normal(0.0, 0.0, 1.0); // 设置填充平面的法向量
				pHatch->setNormal(normal);
				pHatch->setElevation(0.0); // 设置填充平面的高度（与当前Z坐标相关）

				pHatch->setAssociative(false); // 非关联填充
				pHatch->setPattern(AcDbHatch::kPreDefined, TEXT("SOLID")); // 使用预定义的实体填充
				pHatch->setHatchStyle(AcDbHatch::kNormal); // 设置填充样式为普通

				// 添加填充区域

				pHatch->appendLoop(AcDbHatch::kExternal, fillPoints, vertexBulges);

				// 完成填充计算
				pHatch->evaluateHatch();

				pHatch->worldDraw(mode);

				delete pHatch;

				mode->subEntityTraits().setColor(7); // 设置为白色，颜色索引为7

			}
			else if (status[i][j] == 1) { // 已点开的格子
				if (ui[i][j] == -1) { // 如果是雷
					if (1) {
						// 设置灰色填充
						mode->subEntityTraits().setColor(1); // 设置为灰色，颜色索引为8

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
						AcGeVector3d normal(0.0, 0.0, 1.0); // 设置填充平面的法向量
						pHatch->setNormal(normal);
						pHatch->setElevation(0.0); // 设置填充平面的高度（与当前Z坐标相关）

						pHatch->setAssociative(false); // 非关联填充
						pHatch->setPattern(AcDbHatch::kPreDefined, TEXT("SOLID")); // 使用预定义的实体填充
						pHatch->setHatchStyle(AcDbHatch::kNormal); // 设置填充样式为普通

						// 添加填充区域

						pHatch->appendLoop(AcDbHatch::kExternal, fillPoints, vertexBulges);

						// 完成填充计算
						pHatch->evaluateHatch();

						pHatch->worldDraw(mode);

						delete pHatch;

						mode->subEntityTraits().setColor(7); // 设置为白色，颜色索引为7
					}


					// 设置红色填充圆
					mode->subEntityTraits().setColor(250); // 红色的颜色索引是1
					//mode->geometry().circle(AcGePoint3d(i + 0.5, j + 0.5, 0), 0.3, AcGeVector3d::kZAxis);

					   // 构建一个圆
					AcGePoint2d cenPt(i + 0.5, j + 0.5);
					double TWOPI = 2.0 * 3.1415926535897932;
					AcGeCircArc2d *cirArc = new AcGeCircArc2d();
					cirArc->setCenter(cenPt);
					cirArc->setRadius(0.3);
					cirArc->setAngles(0.0, TWOPI);

					AcDbHatch* pHatch = new AcDbHatch();
					AcGeVector3d normal(0.0, 0.0, 1.0); // 设置填充平面的法向量
					pHatch->setNormal(normal);
					pHatch->setElevation(0.0); // 设置填充平面的高度（与当前Z坐标相关）

					pHatch->setAssociative(false); // 非关联填充
					pHatch->setPattern(AcDbHatch::kPreDefined, TEXT("SOLID")); // 使用预定义的实体填充
					pHatch->setHatchStyle(AcDbHatch::kNormal); // 设置填充样式为普通

					// 将圆环添加到填充边界
					AcGeIntArray edgeTypes;
					AcGeVoidPointerArray edgePtrs;
					edgeTypes.append(AcDbHatch::kCirArc);
					edgePtrs.append((void*)cirArc);
					pHatch->appendLoop(AcDbHatch::kDefault, edgePtrs, edgeTypes);


					// 完成填充计算
					pHatch->evaluateHatch();

					pHatch->worldDraw(mode);

					delete pHatch;

					mode->subEntityTraits().setColor(7); // 设置为白色，颜色索引为7
				}
				else { // 显示周围的雷数
					std::wstring number = std::to_wstring(ui[i][j]);

					// 调整文本大小和位置
					double textHeight = 0.6; // 调整文本的高度，根据需要进行调整
					AcGePoint3d textPosition(i + 0.2, j + 0.2, 0); // 文本起始点位置	

					mode->subEntityTraits().setColor(3); // 绿色的颜色索引为3
					mode->geometry().text(
						textPosition,                       // 文本起始点位置
						AcGeVector3d::kZAxis,               // 文本平面的法线方向
						AcGeVector3d::kXAxis,               // 文本方向
						textHeight,                         // 文本高度
						0.0,                                // 文本宽度（默认为0，表示使用标准宽度）
						0.0,                                // 文本斜体角度（默认为0，表示无斜体）
						number.c_str()                      // 要显示的文本内容
					);

					mode->subEntityTraits().setColor(7); // 设置为白色，颜色索引为7
				}
			}
			else if (status[i][j] == 2) { // 已标记的格子

				// 设置灰色填充
				mode->subEntityTraits().setColor(8); // 设置为灰色，颜色索引为8

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
				AcGeVector3d normal(0.0, 0.0, 1.0); // 设置填充平面的法向量
				pHatch->setNormal(normal);
				pHatch->setElevation(0.0); // 设置填充平面的高度（与当前Z坐标相关）

				pHatch->setAssociative(false); // 非关联填充
				pHatch->setPattern(AcDbHatch::kPreDefined, TEXT("SOLID")); // 使用预定义的实体填充
				pHatch->setHatchStyle(AcDbHatch::kNormal); // 设置填充样式为普通

				// 添加填充区域

				pHatch->appendLoop(AcDbHatch::kExternal, fillPoints, vertexBulges);

				// 完成填充计算
				pHatch->evaluateHatch();

				pHatch->worldDraw(mode);

				delete pHatch;

				mode->subEntityTraits().setColor(7); // 设置为白色，颜色索引为7



				//// 设置红色圆框
				//mode->subEntityTraits().setColor(1);
				//// 绘制圆
				//mode->geometry().circle(AcGePoint3d(i + 0.5, j + 0.5, 0), 0.3, AcGeVector3d::kZAxis);

				if (1) {
					// 设置红色填充圆
					mode->subEntityTraits().setColor(1); // 红色的颜色索引是1
					//mode->geometry().circle(AcGePoint3d(i + 0.5, j + 0.5, 0), 0.3, AcGeVector3d::kZAxis);

					   // 构建一个圆
					AcGePoint2d cenPt(i + 0.5, j + 0.5);
					double TWOPI = 2.0 * 3.1415926535897932;
					AcGeCircArc2d *cirArc = new AcGeCircArc2d();
					cirArc->setCenter(cenPt);
					cirArc->setRadius(0.3);
					cirArc->setAngles(0.0, TWOPI);

					AcDbHatch* pHatch = new AcDbHatch();
					AcGeVector3d normal(0.0, 0.0, 1.0); // 设置填充平面的法向量
					pHatch->setNormal(normal);
					pHatch->setElevation(0.0); // 设置填充平面的高度（与当前Z坐标相关）

					pHatch->setAssociative(false); // 非关联填充
					pHatch->setPattern(AcDbHatch::kPreDefined, TEXT("SOLID")); // 使用预定义的实体填充
					pHatch->setHatchStyle(AcDbHatch::kNormal); // 设置填充样式为普通

					// 将圆环添加到填充边界
					AcGeIntArray edgeTypes;
					AcGeVoidPointerArray edgePtrs;
					edgeTypes.append(AcDbHatch::kCirArc);
					edgePtrs.append((void*)cirArc);
					pHatch->appendLoop(AcDbHatch::kDefault, edgePtrs, edgeTypes);


					// 完成填充计算
					pHatch->evaluateHatch();

					pHatch->worldDraw(mode);

					delete pHatch;


				}

				mode->subEntityTraits().setColor(7); // 设置为白色，颜色索引为7
			}
			mode->subEntityTraits().setColor(7);
		}
	}

	
	/*这段是在定义结果显示文字*/
	if (result == 1) {
		std::wstring text = L"超绝扫雷大王黄玉宁！";

		// 调整文本大小和位置
		double textHeight = 2.5; // 调整文本的高度，根据需要进行调整
		AcGePoint3d textPosition(-7, 5, 0); // 文本起始点位置	

		mode->subEntityTraits().setColor(7);
		mode->geometry().text(
			textPosition,                       // 文本起始点位置
			AcGeVector3d::kZAxis,               // 文本平面的法线方向
			AcGeVector3d::kXAxis,               // 文本方向
			textHeight,                         // 文本高度
			0.0,                                // 文本宽度（默认为0，表示使用标准宽度）
			0.0,                                // 文本斜体角度（默认为0，表示无斜体）
			text.c_str()                      // 要显示的文本内容
		);
	}
	else if (result == 2) {
		std::wstring text = L"你输了！";

		// 调整文本大小和位置
		double textHeight = 4; // 调整文本的高度，根据需要进行调整
		AcGePoint3d textPosition(-1, 5, 0); // 文本起始点位置	

		mode->subEntityTraits().setColor(7);
		mode->geometry().text(
			textPosition,                       // 文本起始点位置
			AcGeVector3d::kZAxis,               // 文本平面的法线方向
			AcGeVector3d::kXAxis,               // 文本方向
			textHeight,                         // 文本高度
			0.0,                                // 文本宽度（默认为0，表示使用标准宽度）
			0.0,                                // 文本斜体角度（默认为0，表示无斜体）
			text.c_str()                      // 要显示的文本内容
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
