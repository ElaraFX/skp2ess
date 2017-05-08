#ifndef S2E_MATERIAL_H
#define S2E_MATERIAL_H
#include <SketchUpAPI/model/model.h>
#include <SketchUpAPI/model/entities.h>
#include <SketchUpAPI/model/material.h>
#include <SketchUpAPI/model/texture.h>
#include <SketchUpAPI/color.h>
#include <SketchUpAPI/common.h>
#include <vector>


#define SU_CALL(func) if ((func) != SU_ERROR_NONE) throw std::exception()

struct MaterialInfo {
	MaterialInfo()
		: has_color_(false), has_alpha_(false), alpha_(0.0),
		has_texture_(false), texture_sscale_(0.0), texture_tscale_(0.0) {}

	std::string name_;
	bool has_color_;
	SUColor color_;
	bool has_alpha_;
	double alpha_;
	bool has_texture_;
	std::string texture_path_;
	double texture_sscale_;
	double texture_tscale_;
};

struct MaterialContainer {
	std::vector<MaterialInfo> materialinfos;

	MaterialContainer()
	{
		materialinfos.clear();
	}

	void AddMaterial(MaterialInfo &materialinfo)
	{
		materialinfos.push_back(materialinfo);
	}

	int FindIndexWithString(std::string name)
	{
		for (int i = 0; i < materialinfos.size(); i++)
		{
			if (name == materialinfos[i].name_)
			{
				return i;
			}
		}

		return -1;
	}
};
extern MaterialContainer g_material_container;

class CSUString {
public:
	CSUString() {
		SUSetInvalid(su_str_);
		SUStringCreate(&su_str_);
	}

	~CSUString() {
		SUStringRelease(&su_str_);
	}

	operator SUStringRef*() {
		return &su_str_;
	}

	std::string utf8() {
		size_t length;
		SUStringGetUTF8Length(su_str_, &length);
		std::string string;
		string.resize(length + 1);
		size_t returned_length;
		SUStringGetUTF8(su_str_, length, &string[0], &returned_length);
		return string;
	}

private:
	// Disallow copying for simplicity
	CSUString(const CSUString& copy);
	CSUString& operator= (const CSUString& copy);

	SUStringRef su_str_;
};

extern void GetAllMaterials(SUModelRef model);


#endif