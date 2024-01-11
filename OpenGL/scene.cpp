#include "scene.h"

Scene::Scene() {

}

void Scene::render(Shader shader) {
	for (int i = 0; i < objects.size(); i++)
	{
		model = glm::mat4(1.0f);
		model = glm::translate(model, positions[i]);
		shader.setMat4("model", model);
		objects[i].Draw(shader);
	}
}

void Scene::addObject(const Model& object,const glm::vec3& position) {
	objects.push_back(object);
	positions.push_back(position);
}