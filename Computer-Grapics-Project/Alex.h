#pragma once
#include "Character.h"
#include "Map.h"
#include "Snow.h"


namespace Alex {
	class Character {
	public:
		Part gBody, gHead, gArmL, gArmR, gLegL, gLegR, gBoundingBox;
		GLuint textureID;
		glm::vec3 pos = { 4.5f, 1.5f, 12.0f };
		glm::vec2 moveDir = { 0.0f, 0.0f }; 
		GLfloat moveSpeed = 0.1f; 
		GLfloat throwingSpeed = 1.0f; 
		GLint armState = 0; // 0: IDLE, 1: RUN, 2: CHARGE, 3: LOWERING
		GLfloat armAngle = 0.0f; // 라디안 사용
		GLfloat armDir = 1.0f; 
		GLint legState = 0; // 0: IDLE, 1: RUN
		GLfloat legAngle = 0.0f; 
		GLfloat legDir = 1.0f; 
		glm::vec3 boundingBoxSize; 

		Character(const char* texturePath) {
			textureID = Init::loadTexture(texturePath);

			const float gap = 0.02f; 
			glm::vec3 bodyH(0.3f, 0.35f, 0.15f);        
			glm::vec3 headH(0.25f, 0.25f, 0.25f);       
			glm::vec3 armH(0.09f, 0.35f, 0.12f);       
			glm::vec3 legH(0.12f, 0.35f, 0.12f);        

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
			glm::vec3 legLOffset(-0.12f, -(bodyH.y + legH.y + gap), 0); 
			glm::vec3 legROffset(0.12f, -(bodyH.y + legH.y + gap), 0);
			gLegL = Init::makeCubePart(legH, legLOffset, legLUVs, headBodyLegFlips, glm::vec3(0, +legH.y, 0));
			gLegR = Init::makeCubePart(legH, legROffset, legRUVs, headBodyLegFlips, glm::vec3(0, +legH.y, 0));

			// Bounding Box
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
					if (newState == 0 || newState == 1) armAngle = 0.0f;
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

		void enterCharge() {
			armState = 2; // CHARGE
			armAngle = 0.0f;
		}

		void enterThrow() {
			armState = 3; // LOWERING
		}


		bool checkCollision(const glm::vec3& characterMin, const glm::vec3& characterMax, const Block& block) {
			float blockSize = block.getSize();
			glm::vec3 blockMin = { block.getX() - blockSize / 2.0f, block.getY() - blockSize / 2.0f, block.getZ() - blockSize / 2.0f };
			glm::vec3 blockMax = { block.getX() + blockSize / 2.0f, block.getY() + blockSize / 2.0f, block.getZ() + blockSize / 2.0f };

			bool collisionX = characterMin.x <= blockMax.x && characterMax.x >= blockMin.x;
			bool collisionY = characterMin.y <= blockMax.y && characterMax.y >= blockMin.y;
			bool collisionZ = characterMin.z <= blockMax.z && characterMax.z >= blockMin.z;

			return collisionX && collisionY && collisionZ;
		}

		bool isCollidingWithObstacles(const glm::vec3& nextPos, const Map& map) {
			glm::vec3 halfSize = boundingBoxSize / 2.0f;
			glm::vec3 characterMin = nextPos + gBoundingBox.offset - halfSize;
			glm::vec3 characterMax = nextPos + gBoundingBox.offset + halfSize;

			const Wall& wall = map.getWall();
			for (size_t i = 0; i < wall.getBlockCount(); ++i) {
				if (checkCollision(characterMin, characterMax, wall.getBlock(i))) {
					return true;
				}
			}

			return false; 
		}

		int isCollidingWithSnow(const glm::vec3& nextPos, const Snow& snow) {
			glm::vec3 halfSize = boundingBoxSize / 2.0f;
			glm::vec3 characterMin = nextPos + gBoundingBox.offset - halfSize;
			glm::vec3 characterMax = nextPos + gBoundingBox.offset + halfSize;

			int minGridX = static_cast<int>(floor(characterMin.x));
			int maxGridX = static_cast<int>(ceil(characterMax.x));
			int minGridZ = static_cast<int>(floor(characterMin.z));
			int maxGridZ = static_cast<int>(ceil(characterMax.z));

			for (int gx = minGridX; gx <= maxGridX; ++gx) {
				for (int gz = minGridZ; gz <= maxGridZ; ++gz) {
					float snowHeight = snow.getSnowHeightAt(gx, gz);
					if (snowHeight > 0) {
						glm::vec3 blockMin = { gx - 0.5f, 0.0f, gz - 0.5f };
						glm::vec3 blockMax = { gx + 0.5f, snowHeight, gz + 0.5f };

						bool collisionX = characterMin.x <= blockMax.x && characterMax.x >= blockMin.x;
						bool collisionZ = characterMin.z <= blockMax.z && characterMax.z >= blockMin.z;

						if (collisionX && collisionZ) {
							return snowHeight >= 1.0f ? 2 : 1; 
						}
					}
				}
			}

			return 0; 
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
				return true; 
			}

			return false; 
		}

		void update(const Map& map, const Snow& snow) {
			// 1) 상태 전환 (팔/다리)
			const bool isMoving = (moveDir.x != 0.0f || moveDir.y != 0.0f);
			if (isMoving) {
				if (armState < 2) changeState(0, 1); // RUN
				changeState(1, 1); // leg RUN
			}
			else {
				if (armState < 2) changeState(0, 0); // IDLE
				changeState(1, 0); // leg IDLE
			}

			// 2) 이동 속도 계산 (눈 충돌에 따른 감속 + 상한 클램프)
			int snowTypeAtPos = isCollidingWithSnow(pos, snow);
			float currentMoveSpeed = moveSpeed;
			if (snowTypeAtPos == 1) currentMoveSpeed *= 0.5f; // 얕은 눈 감속
			const float maxMoveSpeed = 1.5f;
			if (currentMoveSpeed > maxMoveSpeed) currentMoveSpeed = maxMoveSpeed;

			// 3) 입력 벡터 정규화
			glm::vec2 dir = moveDir;
			float len = glm::length(dir);
			if (len > 1e-4f) dir /= len;

			// 4) 축별로 충돌 검사하며 이동
			glm::vec3 nextPos = pos;

			// X축 이동
			nextPos.x += dir.x * currentMoveSpeed;
			bool canMoveX = (isCollidingWithSnow(nextPos, snow) < 2) &&
				(!isCollidingWithObstacles(nextPos, map)) &&
				(!isOutsideBackGround(nextPos, map));
			if (canMoveX) {
				pos.x = nextPos.x;
			}

			// Z축 이동
			nextPos = pos;
			nextPos.z += dir.y * currentMoveSpeed;
			bool canMoveZ = (isCollidingWithSnow(nextPos, snow) < 2) &&
				(!isCollidingWithObstacles(nextPos, map)) &&
				(!isOutsideBackGround(nextPos, map));
			if (canMoveZ) {
				pos.z = nextPos.z;
			}

			// 5) 팔 상태별 애니메이션
			switch (armState) {
			case 0: // IDLE
				// 팔 각도 유지
				break;
			case 1: // RUN
				armAngle += armDir * 0.05f;
				if (armAngle > glm::radians(60.0f) || armAngle < glm::radians(-60.0f))
					armDir = -armDir;
				break;
			case 2: // CHARGE
				// 팔 각도는 TimerFunction에서 차징 비율로 -π * ratio 로 갱신됨
				break;
			case 3: // LOWERING: 빠르게 0으로 복귀, 복귀 완료 전까지 재차징 불가
			{
				const float lowerSpeed = 0.25f;
				armAngle += lowerSpeed;
				if (armAngle >= 0.0f) {
					armAngle = 0.0f;
					// 완료 후 팔 상태를 이동 상태에 맞춰 복귀
					if (isMoving) changeState(0, 1);
					else changeState(0, 0);
				}
			}
			break;
			}

			// 6) 다리 애니메이션
			switch (legState) {
			case 0: // IDLE
				// 다리 각도 유지
				break;
			case 1: // RUN
				legAngle += legDir * 0.05f;
				if (legAngle > glm::radians(60.0f) || legAngle < glm::radians(-60.0f))
					legDir = -legDir;
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
			Mbase = glm::rotate(Mbase, glm::radians(180.0f), glm::vec3(0, 1, 0)); // Alex는 뒤쪽을 향함

			float alArm = armAngle;
			float arArm = -alArm;
			if (armState >= 2) arArm = 0.0f;

			float aLeg = std::sin(legAngle) * glm::radians(60.0f);
			aLeg *= -1.0f;

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
		}
	};
}