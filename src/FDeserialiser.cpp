#include "FDeserialiser.h"

#include "FJsonParser.h"
#include "FComponent.h"
#include "FMesh.h"
#include "FPhysicsComponent.h"
#include "FParticlePhysicsComponent.h"
#include "FRigidBodyPhysicsComponent.h"
#include "FObject.h"
#include "FScene.h"
#include "FResourceManager.h"
#include "FDebug.h"
#include "FSphereCollider.h"
#include "FAABBCollider.h"

using namespace std;

bool operator>>(const FJsonElement& a, XMFLOAT3& other)
{
	if (a.type != JARRAY) return false;
	if (a.a_val.size() != 3) return false;

	if (a.a_val[0].type == JFLOAT) other.x = a.a_val[0].f_val;
	if (a.a_val[1].type == JFLOAT) other.y = a.a_val[1].f_val;
	if (a.a_val[2].type == JFLOAT) other.z = a.a_val[2].f_val;

	return true;
}

bool operator>>(const FJsonElement& a, XMFLOAT4& other)
{
	if (a.type != JARRAY) return false;
	if (a.a_val.size() != 4) return false;

	if (a.a_val[0].type == JFLOAT) other.x = a.a_val[0].f_val;
	if (a.a_val[1].type == JFLOAT) other.y = a.a_val[1].f_val;
	if (a.a_val[2].type == JFLOAT) other.z = a.a_val[2].f_val;
	if (a.a_val[3].type == JFLOAT) other.w = a.a_val[3].f_val;

	return true;
}

bool operator>>(const FJsonElement& a, XMINT3& other)
{
	if (a.type != JARRAY) return false;
	if (a.a_val.size() != 3) return false;

	if (a.a_val[0].type == JFLOAT) other.x = (INT)a.a_val[0].f_val;
	if (a.a_val[1].type == JFLOAT) other.y = (INT)a.a_val[1].f_val;
	if (a.a_val[2].type == JFLOAT) other.z = (INT)a.a_val[2].f_val;

	return true;
}

bool deserialiseComponent(const FJsonElement& a, FComponent*& other, FObject* object)
{
    if (a.type != JOBJECT) return false;

	FResourceManager* rm = FResourceManager::get();

    FJsonObject* obj = a.o_val;
    if (obj == nullptr) return false;
    if (!obj->has("class", JSTRING)) return false;
    if (other != nullptr) delete other;
	if (object == nullptr) return false;

	string object_class = (*obj)["class"].s_val;
	if (object_class == "mesh")
	{
        FMeshComponent* other_mesh = new FMeshComponent(object);
		other = other_mesh;
		if (obj->has("data", JSTRING)) other_mesh->setData(rm->loadMesh((*obj)["data"].s_val));
		if (obj->has("material", JSTRING)) other_mesh->setMaterial(rm->loadMaterial((*obj)["material"].s_val));
		if (obj->has("cast_shadow", JFLOAT)) other_mesh->cast_shadow = (*obj)["cast_shadow"].f_val > 0.0f;
	}
	else if (object_class == "camera")
	{
        FCameraComponent* other_camera = new FCameraComponent(object);
		other = other_camera;
		if (obj->has("aspect_ratio", JFLOAT)) other_camera->aspect_ratio = (*obj)["aspect_ratio"].f_val;
		if (obj->has("field_of_view", JFLOAT)) other_camera->field_of_view = (*obj)["field_of_view"].f_val;
		if (obj->has("near_clip", JFLOAT)) other_camera->near_clip = (*obj)["near_clip"].f_val;
		if (obj->has("far_clip", JFLOAT)) other_camera->far_clip = (*obj)["far_clip"].f_val;
		other_camera->updateProjectionMatrix();
	}
	else if (object_class == "empty")
		other = new FComponent((FObject*)object);
	else if (object_class == "light")
	{
        FLightComponent* other_light = new FLightComponent(object);
		other = other_light;
		if (obj->has("colour", JARRAY)) (*obj)["colour"] >> other_light->colour;
		if (obj->has("strength", JFLOAT)) other_light->strength = (*obj)["strength"].f_val;
		if (obj->has("angle", JFLOAT)) other_light->angle = (*obj)["angle"].f_val;
		if (obj->has("type", JSTRING))
        {
            string type = (*obj)["type"].s_val;
            if (type == "directional")
                other_light->type = FLightComponent::FLightType::DIRECTIONAL;
            else if (type == "spot")
                other_light->type = FLightComponent::FLightType::SPOT;
            else if (type == "POINT")
                other_light->type = FLightComponent::FLightType::POINT;
            else return false;
        }
	}
	else if (object_class == "physics")
	{
		FRigidBodyPhysicsComponent* other_physics = new FRigidBodyPhysicsComponent(object);
		other = other_physics;

		if (obj->has("mass", JFLOAT)) other_physics->setMass((*obj)["mass"].f_val);
		if (obj->has("velocity", JARRAY)) { FVector v; (*obj)["velocity"] >> v; other_physics->setVelocity(v); }
		if (obj->has("gravity", JFLOAT)) { other_physics->obeys_gravity = (*obj)["gravity"].f_val > 0.0f; }
		if (obj->has("kinematic", JFLOAT)) { other_physics->kinematic = (*obj)["kinematic"].f_val > 0.0f; }
		if (obj->has("collider", JSTRING))
		{
			string collider_type = (*obj)["collider"].s_val;
			if (collider_type == "sphere")
			{
				FSphereCollider* coll = new FSphereCollider(other_physics);
				other_physics->setCollider(coll);
				if (obj->has("radius", JFLOAT)) coll->setRadius((*obj)["radius"].f_val);
				if (obj->has("center", JARRAY)) { FVector tmp; (*obj)["center"].a_val >> tmp; coll->setCenter(tmp); }
			}
			else if (collider_type == "aabb")
			{
				FAABBCollider* coll = new FAABBCollider(other_physics);
				other_physics->setCollider(coll);
				FVector origin = FVector(0, 0, 0);
				FVector extent = FVector(1, 1, 1);
				if (obj->has("origin", JARRAY)) (*obj)["origin"].a_val >> origin;
				if (obj->has("extent", JARRAY)) (*obj)["extent"].a_val >> extent;
				coll->setBounds(FBoundingBox(origin, extent));
			}
		}
	}
	else
		return false;

    return true;
}

bool deserialiseObject(const FJsonElement& a, FObject*& other, FScene* scene)
{
	if (a.type != JOBJECT) return false;

	FJsonObject* obj = a.o_val;
	if (obj == nullptr) return false;
	if (other == nullptr) return false;
	if (scene == nullptr) return false;

    XMFLOAT3 tmp;

	if (obj->has("name", JSTRING)) other->name = (*obj)["name"].s_val;
	if (obj->has("position", JARRAY)) { (*obj)["position"] >> tmp; other->transform.setLocalPosition(tmp); }
	else other->transform.setLocalPosition(FVector(0, 0, 0));
	if (obj->has("rotation", JARRAY)) { (*obj)["rotation"] >> tmp; other->transform.setLocalEuler(tmp); }
	if (obj->has("scale", JARRAY)) { (*obj)["scale"] >> tmp; other->transform.setLocalScale(tmp); }

	if (obj->has("components", JARRAY))
	{
		vector<FJsonElement> comps = (*obj)["components"].a_val;
		for (FJsonElement comp : comps)
		{
			FComponent* c = nullptr;
			if (deserialiseComponent(comp, c, other))
				other->addComponent(c);
		}
	}

	if (obj->has("children", JARRAY))
	{
		vector<FJsonElement> childs = (*obj)["children"].a_val;
		for (FJsonElement child : childs)
		{
			FObject* child_object = new FObject();
			if (deserialiseObject(child, child_object, scene))
				scene->addObject(child_object, other);
		}
	}

	return true;
}

bool deserialiseScene(const FJsonElement& a, FScene* other)
{
	if (a.type != JOBJECT) return false;

	if (other == nullptr) return false;

	FJsonObject* scene_obj = a.o_val;
	if (scene_obj == nullptr) return false;
	if (scene_obj->has("name", JSTRING)) other->name = (*scene_obj)["name"].s_val;
	if (scene_obj->has("objects", JARRAY))
	{
		for (FJsonElement obj : (*scene_obj)["objects"].a_val)
		{
			FObject* object = new FObject();
			if (deserialiseObject(obj, object, other))
				other->addObject(object, nullptr);
		}
	}
	if (scene_obj->has("ambient_light", JARRAY))
		(*scene_obj)["ambient_light"] >> other->ambient_light;
	if (scene_obj->has("fog_colour", JARRAY))
		(*scene_obj)["fog_colour"] >> other->fog_colour;
	if (scene_obj->has("fog_start", JFLOAT))
		other->fog_start = (*scene_obj)["fog_start"].f_val;
	if (scene_obj->has("fog_end", JFLOAT))
		other->fog_end = (*scene_obj)["fog_end"].f_val;
	if (scene_obj->has("fog_strength", JFLOAT))
		other->fog_strength = (*scene_obj)["fog_strength"].f_val;

	return true;
}