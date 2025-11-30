#pragma once
#include "Character.h"
#include "Map.h"

namespace Alex {
	class Character {
	public:
		Part gBody, gHead, gArmL, gArmR, gLegL, gLegR, gBoundingBox;
		GLuint textureID;
		glm::vec3 pos = { 4.5f, 1.5f, 12.0f };
		glm::vec2 moveDir = { 0.0f, 0.0f }; // 이동 방향 벡터 x, z
		GLfloat moveSpeed = 0.1f; // 이동 속도
		GLfloat throwingSpeed = 0.2f; // 투사체 속도
		GLint armState = 0; // 0: IDLE, 1: RUN, 2: CHARGE 3: THROW
		GLfloat armAngle = 0.0f; // 팔 회전 각도
		GLfloat armDir = 1.0f; // 팔 회전 방향
		GLint legState = 0; // 0: IDLE, 1: RUN
		GLfloat legAngle = 0.0f; // 다리 회전 각도
		GLfloat legDir = 1.0f; // 다리 회전 방향
		glm::vec3 boundingBoxSize; // 바운딩 박스 크기 저장

		Character(const char* texturePath) {
			textureID = Init::loadTexture(texturePath);

			const float gap = 0.02f; // 간격을 더 줄임 (0.03f -> 0.02f)
			
			// 높이를 더욱 줄임 (전체 높이 약 1.5 정도로)
			glm::vec3 bodyH(0.3f, 0.35f, 0.15f);        // 몸통: (0.35, 0.5, 0.17) -> (0.3, 0.35, 0.15)
			glm::vec3 headH(0.25f, 0.25f, 0.25f);       // 머리: (0.33, 0.33, 0.33) -> (0.25, 0.25, 0.25)
			glm::vec3 armH(0.09f, 0.35f, 0.12f);        // Alex의 얇은 팔: (0.125, 0.5, 0.17) -> (0.09, 0.35, 0.12)
			glm::vec3 legH(0.12f, 0.35f, 0.12f);        // 다리: (0.17, 0.5, 0.17) -> (0.12, 0.35, 0.12)

			UVRect headUVs[] = { {0,8,8,8}, {16,8,8,8}, {8,0,8,8}, {16,0,8,8}, {8,8,8,8}, {24,8,8,8} };
			UVRect bodyUVs[] = { {16,20,4,12}, {28,20,4,12}, {20,16,8,4}, {28,16,8,4}, {20,20,8,12}, {32,20,8,12} };
			UVRect armLUVs[] = { {40,20,4,12}, {52,20,4,12}, {44,16,4,4}, {48,16,4,4}, {44,20,4,12}, {52,20,4,12} };
			UVRect armRUVs[] = { {40,20,4,12}, {52,20,4,12}, {44,16,4,4}, {48,16,4,4}, {44,20,4,12}, {52,20,4,12} };
			UVRect legLUVs[] = { {0,20,4,12}, {8,20,4,12}, {4,16,4,4}, {8,16,4,4}, {4,20,4,12}, {12,20,4,12} };
			UVRect legRUVs[] = { {0,20,4,12}, {8,20,4,12}, {4,16,4,4}, {8,16,4,4}, {4,20,4,12}, {12,20,4,12} };

			const bool headBodyLegFlips[] = { true, true, false, false, true, true };
			const bool armFlips[] = { true, true, false, false, true, false };

			gBody = Init::makeCubePart(bodyH, glm::vec3(0, 0, 0), bodyUVs, headBodyLegFlips, glm::vec3(0));
			glm::vec3 headOffset(0, bodyH.y + headH.y + gap, 0);
			gHead = Init::makeCubePart(headH, headOffset, headUVs, headBodyLegFlips, glm::vec3(0, -headH.y, 0));
			glm::vec3 armLOffset(-(bodyH.x + armH.x + gap), bodyH.y - armH.y, 0);
			glm::vec3 armROffset(+(bodyH.x + armH.x + gap), bodyH.y - armH.y, 0);
			gArmL = Init::makeCubePart(armH, armLOffset, armLUVs, armFlips, glm::vec3(0, +armH.y, 0));
			gArmR = Init::makeCubePart(armH, armROffset, armRUVs, armFlips, glm::vec3(0, +armH.y, 0));
			glm::vec3 legLOffset(-0.12f, -(bodyH.y + legH.y + gap), 0); // 다리 간격 더 줄임 (0.17f -> 0.12f)
			glm::vec3 legROffset(0.12f, -(bodyH.y + legH.y + gap), 0);
			gLegL = Init::makeCubePart(legH, legLOffset, legLUVs, headBodyLegFlips, glm::vec3(0, +legH.y, 0));
			gLegR = Init::makeCubePart(legH, legROffset, legRUVs, headBodyLegFlips, glm::vec3(0, +legH.y, 0));

			// Bounding Box 재계산 (더 줄어든 크기에 맞춰)
			float top = headOffset.y + headH.y;
			float bottom = legLOffset.y - legH.y;
			float front = headOffset.z + headH.z;
			float back = headOffset.z - headH.z;
			float left = gBody.offset.x - bodyH.x;
			float right = gBody.offset.x + bodyH.x;
			boundingBoxSize = glm::vec3(right - left, top - bottom, front - back);
			gBoundingBox = Init::makeLineCubePart(right - left, top - bottom, front - back);
			gBoundingBox.offset = glm::vec3((left + right) / 2.0f, (top + bottom) / 2.0f, (front + back) / 2.0f);
		}

		void changeState(int body, int newState) {
			if (body == 0) {
				if (armState != newState) {
					armState = newState; // arm
					armAngle = 0.0f;
					if (newState == 1) armDir = 1.0f; // RUN
				}
			}
			else {
				if (legState != newState) {
					legState = newState; // leg
					legAngle = 0.0f;
					if (newState == 1) legDir = -1.0f; // RUN
				}
			}
		}

		void enterThrow() {
			armState = 2; // THROW
			armAngle = glm::radians(-90.0f);
		}

		// AABB 충돌 검사 함수
		bool checkCollision(const glm::vec3& characterMin, const glm::vec3& characterMax, const Block& block) {
			float blockSize = block.getSize();
			glm::vec3 blockMin = { block.getX() - blockSize / 2.0f, block.getY() - blockSize / 2.0f, block.getZ() - blockSize / 2.0f };
			glm::vec3 blockMax = { block.getX() + blockSize / 2.0f, block.getY() + blockSize / 2.0f, block.getZ() + blockSize / 2.0f };

			bool collisionX = characterMin.x <= blockMax.x && characterMax.x >= blockMin.x;
			bool collisionY = characterMin.y <= blockMax.y && characterMax.y >= blockMin.y;
			bool collisionZ = characterMin.z <= blockMax.z && characterMax.z >= blockMin.z;

			return collisionX && collisionY && collisionZ;
		}

		// 장애물과의 충돌 검사
		bool isCollidingWithObstacles(const glm::vec3& nextPos, const Map& map) {
			glm::vec3 halfSize = boundingBoxSize / 2.0f;
			glm::vec3 characterMin = nextPos + gBoundingBox.offset - halfSize;
			glm::vec3 characterMax = nextPos + gBoundingBox.offset + halfSize;

			// 벽과 충돌 검사
			const Wall& wall = map.getWall();
			for (size_t i = 0; i < wall.getBlockCount(); ++i) {
				if (checkCollision(characterMin, characterMax, wall.getBlock(i))) {
					return true;
				}
			}

			return false; // 충돌 없음
		}

		bool isOutsideBackGround(const glm::vec3& nextPos, const Map& map) {
			glm::vec3 halfSize = boundingBoxSize / 2.0f;
			glm::vec3 characterMin = nextPos + gBoundingBox.offset - halfSize;
			glm::vec3 characterMax = nextPos + gBoundingBox.offset + halfSize;

			const Ground& backGround = map.getBackGround();
			const Block& firstBlock = backGround.getBlock(0, 0);
			const Block& lastBlock = backGround.getBlock(backGround.getWidth() - 1, backGround.getDepth() - 1);
			float blockSize = firstBlock.getSize();

			float minX = firstBlock.getX() - blockSize / 2.0f;
			float maxX = lastBlock.getX() + blockSize / 2.0f;
			float minZ = firstBlock.getZ() - blockSize / 2.0f;
			float maxZ = lastBlock.getZ() + blockSize / 2.0f;

			if (characterMin.x < minX || characterMax.x > maxX || characterMin.z < minZ || characterMax.z > maxZ) {
				return true; // 경계 벗어남
			}

			return false; // 경계 내에 있음
		}

		void update(const Map& map) {
			if (moveDir.x == 0.0f && moveDir.y == 0.0f) {
				// 정지 상태
				if (armState < 2) changeState(0, 0); // arm IDLE
				changeState(1, 0); // leg IDLE
			}
			else {
				// 이동 상태
				if (armState < 2) changeState(0, 1); // arm RUN
				changeState(1, 1); // leg RUN
			}

			// 이동 전 충돌 검사
			glm::vec3 nextPos = pos;

			// X축 이동 검사
			nextPos.x += moveDir.x * moveSpeed;
			if (!isCollidingWithObstacles(nextPos, map) && !isOutsideBackGround(nextPos, map)) {
				pos.x = nextPos.x;
			}

			// Z축 이동 검사
			nextPos = pos; // X축 이동이 확정된 위치에서 다시 시작
			nextPos.z += moveDir.y * moveSpeed;
			if (!isCollidingWithObstacles(nextPos, map) && !isOutsideBackGround(nextPos, map)) {
				pos.z = nextPos.z;
			}

			switch (armState) {
			case 0: // IDLE
				break;
			case 1: // RUN
				armAngle += armDir * 0.05f;
				if (armAngle > glm::radians(60.0f) || armAngle < glm::radians(-60.0f)) armDir = -armDir;

				break;
			case 2: // CHARGE
				break;
			case 3: // THROW
				armAngle += 0.01f;

				if (armAngle > -88.0f) {
					armAngle = 0.0f;
					if (legState == 0) changeState(0, 0);
					else changeState(0, 1);
				}
				break;
			}

			switch (legState) {
			case 0: // IDLE
				break;
			case 1: // RUN
				legAngle += legDir * 0.05f;
				if (legAngle > glm::radians(60.0f) || legAngle < glm::radians(-60.0f)) legDir = -legDir;

				break;
			}
		}

		void drawBoundingBox(GLuint modelLoc, const glm::mat4& M) {
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &M[0][0]);
			glBindVertexArray(gBoundingBox.vao);
			glDrawArrays(GL_LINES, 0, gBoundingBox.count);
			glBindVertexArray(0);
		}

		void draw(GLuint modelLoc) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureID);

			auto drawVAO = [&](const Part& p, const glm::mat4& M) {
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &M[0][0]);
				glBindVertexArray(p.vao);
				glDrawArrays(GL_TRIANGLES, 0, p.count);
				glBindVertexArray(0);
				};

			glm::mat4 Mbase = glm::translate(glm::mat4(1.0f), pos);
			Mbase = glm::rotate(Mbase, glm::radians(180.0f), glm::vec3(0, 1, 0)); // Alex가 Z-방향을 바라보도록 회전)

			float alArm = armAngle; // 왼쪽에 보이는 팔 각도

			float arArm = -alArm;
			if (armState == 3) arArm = 0.0f; // 던지는 동작에서는 오른팔이 앞으로 쭉 뻗음

			float aLeg = std::sin(legAngle) * glm::radians(60.0f);
			aLeg *= -1.0f; // 다리는 팔과 반대 방향으로 움직임

			glm::mat4 Mbody = Mbase * glm::translate(glm::mat4(1.0f), gBody.offset);
			drawVAO(gBody, Mbody);

			glm::mat4 Mhead = Mbase * glm::translate(glm::mat4(1.0f), gHead.offset) * glm::translate(glm::mat4(1.0f), gHead.pivot) * glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0, 1, 0)) * glm::translate(glm::mat4(1.0f), -gHead.pivot);
			drawVAO(gHead, Mhead);

			glm::mat4 Marl = Mbase * glm::translate(glm::mat4(1.0f), gArmL.offset) * glm::translate(glm::mat4(1.0f), gArmL.pivot) * glm::rotate(glm::mat4(1.0f), alArm, glm::vec3(1, 0, 0)) * glm::translate(glm::mat4(1.0f), -gArmL.pivot);
			drawVAO(gArmL, Marl);

			glm::mat4 Marr = Mbase * glm::translate(glm::mat4(1.0f), gArmR.offset) * glm::translate(glm::mat4(1.0f), gArmR.pivot) * glm::rotate(glm::mat4(1.0f), arArm, glm::vec3(1, 0, 0)) * glm::translate(glm::mat4(1.0f), -gArmR.pivot);
			drawVAO(gArmR, Marr);

			glm::mat4 Mll = Mbase * glm::translate(glm::mat4(1.0f), gLegL.offset) * glm::translate(glm::mat4(1.0f), gLegL.pivot) * glm::rotate(glm::mat4(1.0f), aLeg, glm::vec3(1, 0, 0)) * glm::translate(glm::mat4(1.0f), -gLegL.pivot);
			drawVAO(gLegL, Mll);

			glm::mat4 Mlr = Mbase * glm::translate(glm::mat4(1.0f), gLegR.offset) * glm::translate(glm::mat4(1.0f), gLegR.pivot) * glm::rotate(glm::mat4(1.0f), -aLeg, glm::vec3(1, 0, 0)) * glm::translate(glm::mat4(1.0f), -gLegR.pivot);
			drawVAO(gLegR, Mlr);

			glm::mat4 Mbounding = Mbase * glm::translate(glm::mat4(1.0f), gBoundingBox.offset);
			drawBoundingBox(modelLoc, Mbounding);
		}
	};
}
