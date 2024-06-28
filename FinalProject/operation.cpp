#include "pch.h"
#include "dbapserv.h"
#include "MineClearance.h"
#include "acedads.h"

void initScreen() {

	// 获取当前文档和数据库
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbBlockTable* pBlockTable;
	pDb->getBlockTable(pBlockTable, AcDb::kForWrite);

	AcDbBlockTableRecord* pBlockTableRecord;
	pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord, AcDb::kForWrite);
	pBlockTable->close();

	// 遍历块表记录中的所有实体并删除它们
	AcDbBlockTableRecordIterator* pIter;
	pBlockTableRecord->newIterator(pIter);

	AcDbEntity* pEntity;
	while (!pIter->done()) {
		pIter->getEntity(pEntity, AcDb::kForWrite);
		pIter->step();
		pEntity->erase();  // 删除实体
		pEntity->close();
	}
	delete pIter;

	pBlockTableRecord->close();
}

AcGePoint3d selectPoint() {
	ads_point point;

	if (acedGetPoint(NULL, NULL, point) != RTNORM) {
		acutPrintf(L"\n未选择任何点.");
		return AcGePoint3d::kOrigin;  // 返回原点表示未选择有效的点
	}
	return AcGePoint3d(point[0], point[1], point[2]);
}


bool isValidCell(int x, int y) {
	// Implement your own validation logic here
	return ((x >= 0 && x < 12 && y >= 0 && y < 12));
}

//递归展开0周围格子的函数
void expandEmptyCells(CMineClearance* pCMineClearance, int x, int y) {
	// Validate the cell coordinates
	if (!isValidCell(x, y) || pCMineClearance->status[x][y] != 0 || pCMineClearance->ui[x][y]==-1) {
		return;
	}

	// Mark the current cell as opened

	pCMineClearance->status[x][y] = 1;

	if (pCMineClearance->ui[x][y] != 0)
		return;

	// Check all neighboring cells (up to 8 directions)
	for (int dx = -1; dx <= 1; dx++) {
		for (int dy = -1; dy <= 1; dy++) {
			if (dx == 0 && dy == 0) continue; // Skip the current cell itself

			int nx = x + dx;
			int ny = y + dy;

			// Recursively expand neighboring cells if they are also empty
			expandEmptyCells(pCMineClearance, nx, ny);
		}
	}
}

void creatMineClearance() {

	initScreen();

	CMineClearance* pCMineClearance = new CMineClearance();

	// 获取当前文档和数据库
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbBlockTable* pBlockTable;
	pDb->getBlockTable(pBlockTable, AcDb::kForWrite);

	AcDbBlockTableRecord* pBlockTableRecord;
	pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord, AcDb::kForWrite);
	pBlockTable->close();

	// 将实体添加到块表记录中
	AcDbObjectId objId;
	pBlockTableRecord->appendAcDbEntity(objId, pCMineClearance);
	
	pCMineClearance->close();

	int gameStatus = 1;
	int roundStatus = 1;
	 // 游戏循环，直到游戏状态改变
	while (gameStatus==1) {

		AcGePoint3d pt = selectPoint();

		//将点坐标转为格点
		int x = static_cast<int>(pt.x);
		int y = static_cast<int>(pt.y);

		bool gameStillOn = false;

		pCMineClearance = nullptr;
		acdbOpenAcDbEntity((AcDbEntity*&)pCMineClearance, objId, AcDb::kForWrite);

		//点击重新开始按钮
		if (x >= 15 && x < 19 && y >= 8 && y <= 9) {

			roundStatus = 1;
			pCMineClearance->init();
			pCMineClearance->recordGraphicsModified();


		}

		//点击结束游戏按钮
		if (x >= 15 && x < 19 && y >= 5 && y <= 6) {

			gameStatus = 0;
			pCMineClearance->recordGraphicsModified();

		}


		if (roundStatus == 1) {		//点击标记地雷按钮
			if (x >= 15 && x < 19 && y >= 11 && y <= 12) {

				pCMineClearance->markStatus = !pCMineClearance->markStatus;
				pCMineClearance->recordGraphicsModified();

			}

			if (isValidCell(x, y)) {
				//非标记状态下
				if (pCMineClearance->markStatus == 0) {

					//未打开的情况
					//不管处于什么状态都打开
					if (pCMineClearance->ui[x][y] == 0) {
						expandEmptyCells(pCMineClearance, x, y);
						pCMineClearance->recordGraphicsModified();
					}
					else {
						pCMineClearance->status[x][y] = 1; // Mark as opened (assuming 1 means opened)
						pCMineClearance->recordGraphicsModified();
					}

					if (pCMineClearance->ui[x][y] == -1)
					{
						pCMineClearance->gameStatus = 0;
						pCMineClearance->result = 2;
						for (int i = 0; i < 12; ++i) {
							for (int j = 0; j < 12; ++j) {
								if (pCMineClearance->ui[i][j] == -1) {
									pCMineClearance->status[i][j] = 1;

								}

							}
						}
						pCMineClearance->recordGraphicsModified();
						acutPrintf(L"本局游戏结束\n");
					}

				}
				//标记状态下
				else
				{
					//未标记
					if (pCMineClearance->status[x][y] == 0) {
						pCMineClearance->status[x][y] = 2;
						pCMineClearance->recordGraphicsModified();
					}
					//已标记
					else if (pCMineClearance->status[x][y] == 2) {
						//取消标记，回到未打开状态
						pCMineClearance->status[x][y] = 0;
						pCMineClearance->recordGraphicsModified();
					}
				}


			}

			else {
				acutPrintf(L"Invalid cell coordinates.\n");
			}


			// 检查是否还有未打开的格子

			for (int i = 0; i < 12; i++) {
				for (int j = 0; j < 12; j++) {
					if (pCMineClearance->status[i][j] == 0 && pCMineClearance->ui[i][j] != -1) {

						gameStillOn = true;
						break;
					}
				}
				if (gameStillOn) break;
			}

			if (!gameStillOn) {
				pCMineClearance->gameStatus = 0;
				pCMineClearance->result = 1;
				pCMineClearance->recordGraphicsModified();
				acutPrintf(L"本局游戏结束\n");
			}

			roundStatus = pCMineClearance->gameStatus;

			pCMineClearance->close();
		}
		


		pCMineClearance->close();
	}


	pBlockTableRecord->close();

}



