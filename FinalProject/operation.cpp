#include "pch.h"
#include "dbapserv.h"
#include "MineClearance.h"
#include "acedads.h"

void initScreen() {

	// ��ȡ��ǰ�ĵ������ݿ�
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbBlockTable* pBlockTable;
	pDb->getBlockTable(pBlockTable, AcDb::kForWrite);

	AcDbBlockTableRecord* pBlockTableRecord;
	pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord, AcDb::kForWrite);
	pBlockTable->close();

	// ��������¼�е�����ʵ�岢ɾ������
	AcDbBlockTableRecordIterator* pIter;
	pBlockTableRecord->newIterator(pIter);

	AcDbEntity* pEntity;
	while (!pIter->done()) {
		pIter->getEntity(pEntity, AcDb::kForWrite);
		pIter->step();
		pEntity->erase();  // ɾ��ʵ��
		pEntity->close();
	}
	delete pIter;

	pBlockTableRecord->close();
}

AcGePoint3d selectPoint() {
	ads_point point;

	if (acedGetPoint(NULL, NULL, point) != RTNORM) {
		acutPrintf(L"\nδѡ���κε�.");
		return AcGePoint3d::kOrigin;  // ����ԭ���ʾδѡ����Ч�ĵ�
	}
	return AcGePoint3d(point[0], point[1], point[2]);
}


bool isValidCell(int x, int y) {
	// Implement your own validation logic here
	return ((x >= 0 && x < 12 && y >= 0 && y < 12));
}

//�ݹ�չ��0��Χ���ӵĺ���
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

	// ��ȡ��ǰ�ĵ������ݿ�
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbBlockTable* pBlockTable;
	pDb->getBlockTable(pBlockTable, AcDb::kForWrite);

	AcDbBlockTableRecord* pBlockTableRecord;
	pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord, AcDb::kForWrite);
	pBlockTable->close();

	// ��ʵ����ӵ�����¼��
	AcDbObjectId objId;
	pBlockTableRecord->appendAcDbEntity(objId, pCMineClearance);
	
	pCMineClearance->close();

	int gameStatus = 1;
	int roundStatus = 1;
	 // ��Ϸѭ����ֱ����Ϸ״̬�ı�
	while (gameStatus==1) {

		AcGePoint3d pt = selectPoint();

		//��������תΪ���
		int x = static_cast<int>(pt.x);
		int y = static_cast<int>(pt.y);

		bool gameStillOn = false;

		pCMineClearance = nullptr;
		acdbOpenAcDbEntity((AcDbEntity*&)pCMineClearance, objId, AcDb::kForWrite);

		//������¿�ʼ��ť
		if (x >= 15 && x < 19 && y >= 8 && y <= 9) {

			roundStatus = 1;
			pCMineClearance->init();
			pCMineClearance->recordGraphicsModified();


		}

		//���������Ϸ��ť
		if (x >= 15 && x < 19 && y >= 5 && y <= 6) {

			gameStatus = 0;
			pCMineClearance->recordGraphicsModified();

		}


		if (roundStatus == 1) {		//�����ǵ��װ�ť
			if (x >= 15 && x < 19 && y >= 11 && y <= 12) {

				pCMineClearance->markStatus = !pCMineClearance->markStatus;
				pCMineClearance->recordGraphicsModified();

			}

			if (isValidCell(x, y)) {
				//�Ǳ��״̬��
				if (pCMineClearance->markStatus == 0) {

					//δ�򿪵����
					//���ܴ���ʲô״̬����
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
						acutPrintf(L"������Ϸ����\n");
					}

				}
				//���״̬��
				else
				{
					//δ���
					if (pCMineClearance->status[x][y] == 0) {
						pCMineClearance->status[x][y] = 2;
						pCMineClearance->recordGraphicsModified();
					}
					//�ѱ��
					else if (pCMineClearance->status[x][y] == 2) {
						//ȡ����ǣ��ص�δ��״̬
						pCMineClearance->status[x][y] = 0;
						pCMineClearance->recordGraphicsModified();
					}
				}


			}

			else {
				acutPrintf(L"Invalid cell coordinates.\n");
			}


			// ����Ƿ���δ�򿪵ĸ���

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
				acutPrintf(L"������Ϸ����\n");
			}

			roundStatus = pCMineClearance->gameStatus;

			pCMineClearance->close();
		}
		


		pCMineClearance->close();
	}


	pBlockTableRecord->close();

}



