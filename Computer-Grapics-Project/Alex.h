#pragma once
#include "Character.h"

namespace Alex {
	class Character {
	public:
		Part gBody, gHead, gArmL, gArmR, gLegL, gLegR;
		GLuint textureID;

		Character(const char* texturePath) {
			textureID = Init::loadTexture(texturePath);

			const float gap = 0.05f;
			glm::vec3 bodyH(0.5f, 0.75f, 0.25f);
			glm::vec3 headH(0.5f, 0.5f, 0.5f);
			glm::vec3 armH(0.1875f, 0.75f, 0.25f); // AlexÀÇ ¾ãÀº ÆÈ
			glm::vec3 legH(0.25f, 0.75f, 0.25f);

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
			glm::vec3 legLOffset(-0.25f, -(bodyH.y + legH.y + gap), 0);
			glm::vec3 legROffset(0.25f, -(bodyH.y + legH.y + gap), 0);
			gLegL = Init::makeCubePart(legH, legLOffset, legLUVs, headBodyLegFlips, glm::vec3(0, +legH.y, 0));
			gLegR = Init::makeCubePart(legH, legROffset, legRUVs, headBodyLegFlips, glm::vec3(0, +legH.y, 0));
		}

		void draw(GLuint modelLoc, glm::mat4 trans) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureID);

			auto drawVAO = [&](const Part& p, const glm::mat4& M) {
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &M[0][0]);
				glBindVertexArray(p.vao);
				glDrawArrays(GL_TRIANGLES, 0, p.count);
				glBindVertexArray(0);
				};

			static float baseAngle = 0.0f;
			baseAngle += 0.5f; if (baseAngle > 360.0f) baseAngle -= 360.0f;
			glm::mat4 Mbase = trans;
			Mbase = glm::rotate(Mbase, glm::radians(baseAngle), glm::vec3(0, 1, 0));

			float t = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
			float aArm = std::sin(t * 2.0f) * glm::radians(35.0f);
			float aLeg = -aArm;

			glm::mat4 Mbody = Mbase * glm::translate(glm::mat4(1.0f), gBody.offset);
			drawVAO(gBody, Mbody);

			glm::mat4 Mhead = Mbase * glm::translate(glm::mat4(1.0f), gHead.offset) * glm::translate(glm::mat4(1.0f), gHead.pivot) * glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0, 1, 0)) * glm::translate(glm::mat4(1.0f), -gHead.pivot);
			drawVAO(gHead, Mhead);

			glm::mat4 Marl = Mbase * glm::translate(glm::mat4(1.0f), gArmL.offset) * glm::translate(glm::mat4(1.0f), gArmL.pivot) * glm::rotate(glm::mat4(1.0f), aArm, glm::vec3(1, 0, 0)) * glm::translate(glm::mat4(1.0f), -gArmL.pivot);
			drawVAO(gArmL, Marl);

			glm::mat4 Marr = Mbase * glm::translate(glm::mat4(1.0f), gArmR.offset) * glm::translate(glm::mat4(1.0f), gArmR.pivot) * glm::rotate(glm::mat4(1.0f), -aArm, glm::vec3(1, 0, 0)) * glm::translate(glm::mat4(1.0f), -gArmR.pivot);
			drawVAO(gArmR, Marr);

			glm::mat4 Mll = Mbase * glm::translate(glm::mat4(1.0f), gLegL.offset) * glm::translate(glm::mat4(1.0f), gLegL.pivot) * glm::rotate(glm::mat4(1.0f), aLeg, glm::vec3(1, 0, 0)) * glm::translate(glm::mat4(1.0f), -gLegL.pivot);
			drawVAO(gLegL, Mll);

			glm::mat4 Mlr = Mbase * glm::translate(glm::mat4(1.0f), gLegR.offset) * glm::translate(glm::mat4(1.0f), gLegR.pivot) * glm::rotate(glm::mat4(1.0f), -aLeg, glm::vec3(1, 0, 0)) * glm::translate(glm::mat4(1.0f), -gLegR.pivot);
			drawVAO(gLegR, Mlr);
		}
	};
}