/*
 * Copyright (c) 2018 The Foundation for Research on Information Technologies in Society (IT'IS).
 * 
 * This file is part of iSEG
 * (see https://github.com/ITISFoundation/osparc-iseg).
 * 
 * This software is released under the MIT License.
 *  https://opensource.org/licenses/MIT
 */
#include "Precompiled.h"

#include "SlicesHandler.h"
#include "TissueHierarchy.h"
#include "TissueInfos.h"

#include "Data/Logger.h"
#include "Data/ScopedTimer.h"

#include "Core/HDF5Reader.h"
#include "Core/HDF5Writer.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <vtkSmartPointer.h>

#include <cctype>
#include <cstdio>
#include <fstream>
#include <iostream>

using namespace iseg;

namespace {
std::string str_tolower(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(),
			[](unsigned char c) { return std::tolower(c); } // correct
	);
	return s;
}
} // namespace

TissueInfo* TissueInfos::GetTissueInfo(tissues_size_t tissuetype)
{
	return &tissueInfosVector[tissuetype];
}

const Color& TissueInfos::GetTissueColor(tissues_size_t tissuetype)
{
	return tissueInfosVector.at(tissuetype).color;
}

std::tuple<std::uint8_t, std::uint8_t, std::uint8_t> iseg::TissueInfos::GetTissueColorMapped(tissues_size_t tissuetype)
{
	if (int(tissuetype) < tissueInfosVector.size())
	{
		return tissueInfosVector[tissuetype].color.toUChar();
	}
	else
	{
		using rgb = std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>;
		if (tissuetype == Mark::RED)
		{
			return rgb(255, 0, 0);
		}
		else if (tissuetype == Mark::GREEN)
		{
			return rgb(0, 255, 0);
		}
		else if (tissuetype == Mark::BLUE)
		{
			return rgb(0, 0, 255);
		}
		else //if (tissuetype == Mark::WHITE)
		{
			return rgb(255, 255, 255);
		}
	}
}

void TissueInfos::GetTissueColorBlendedRGB(tissues_size_t tissuetype, unsigned char& r, unsigned char& g, unsigned char& b, unsigned char offset)
{
	auto& color = tissueInfosVector.at(tissuetype).color;
	auto opac = tissueInfosVector.at(tissuetype).opac;
	r = (unsigned char)(offset + opac * (255.0f * color[0] - offset));
	g = (unsigned char)(offset + opac * (255.0f * color[1] - offset));
	b = (unsigned char)(offset + opac * (255.0f * color[2] - offset));
}

float TissueInfos::GetTissueOpac(tissues_size_t tissuetype)
{
	return tissueInfosVector[tissuetype].opac;
}

std::string TissueInfos::GetTissueName(tissues_size_t tissuetype)
{
	return tissueInfosVector[tissuetype].name;
}

bool TissueInfos::GetTissueLocked(tissues_size_t tissuetype)
{
	if (tissuetype > tissueInfosVector.size() - 1)
		return false;

	return tissueInfosVector[tissuetype].locked;
}

tissues_size_t TissueInfos::GetTissueType(std::string tissuename)
{
	std::string lower = str_tolower(tissuename);
	if (tissueTypeMap.find(lower) != tissueTypeMap.end())
	{
		return tissueTypeMap[lower];
	}
	return 0;
}

void TissueInfos::SetTissueColor(tissues_size_t tissuetype, float r, float g, float b)
{
	tissueInfosVector[tissuetype].color = Color(r, g, b);
}

void TissueInfos::SetTissueOpac(tissues_size_t tissuetype, float val)
{
	tissueInfosVector[tissuetype].opac = val;
}

void TissueInfos::SetTissueName(tissues_size_t tissuetype, std::string val)
{
	tissueTypeMap.erase(str_tolower(tissueInfosVector[tissuetype].name));
	tissueTypeMap.insert(TissueTypeMapEntryType(str_tolower(val), tissuetype));
	tissueInfosVector[tissuetype].name = val;
}

void TissueInfos::SetTissueLocked(tissues_size_t tissuetype, bool val)
{
	tissueInfosVector[tissuetype].locked = val;
}

void TissueInfos::SetTissuesLocked(bool val)
{
	for (auto vecIt = tissueInfosVector.begin() + 1; vecIt != tissueInfosVector.end(); ++vecIt)
	{
		vecIt->locked = val;
	}
}

void TissueInfos::InitTissues()
{
	tissueInfosVector.clear();
	tissueInfosVector.resize(83);

	tissueInfosVector[1].name = "Adrenal_gland";
	tissueInfosVector[1].color = Color(0.338000f, 0.961000f, 0.725000f);
	tissueInfosVector[2].name = "Air_internal";
	tissueInfosVector[2].color = Color(0.000000f, 0.000000f, 0.000000f);
	tissueInfosVector[3].name = "Artery";
	tissueInfosVector[3].color = Color(0.800000f, 0.000000f, 0.000000f);
	tissueInfosVector[4].name = "Bladder";
	tissueInfosVector[4].color = Color(0.529400f, 0.854900f, 0.011800f);
	tissueInfosVector[5].name = "Blood_vessel";
	tissueInfosVector[5].color = Color(0.666700f, 0.003900f, 0.003900f);
	tissueInfosVector[6].name = "Bone";
	tissueInfosVector[6].color = Color(0.929412f, 0.839216f, 0.584314f);
	tissueInfosVector[7].name = "Brain_grey_matter";
	tissueInfosVector[7].color = Color(0.500000f, 0.500000f, 0.500000f);
	tissueInfosVector[8].name = "Brain_white_matter";
	tissueInfosVector[8].color = Color(0.900000f, 0.900000f, 0.900000f);
	tissueInfosVector[9].name = "Breast";
	tissueInfosVector[9].color = Color(0.996000f, 0.741000f, 1.000000f);
	tissueInfosVector[10].name = "Bronchi";
	tissueInfosVector[10].color = Color(0.528000f, 0.592000f, 1.000000f);
	tissueInfosVector[11].name = "Bronchi_lumen";
	tissueInfosVector[11].color = Color(0.368600f, 0.474500f, 0.635300f);
	tissueInfosVector[12].name = "Cartilage";
	tissueInfosVector[12].color = Color(0.627000f, 0.988000f, 0.969000f);
	tissueInfosVector[13].name = "Cerebellum";
	tissueInfosVector[13].color = Color(0.648000f, 0.599000f, 0.838000f);
	tissueInfosVector[14].name = "Cerebrospinal_fluid";
	tissueInfosVector[14].color = Color(0.474500f, 0.521600f, 0.854900f);
	tissueInfosVector[15].name = "Connective_tissue";
	tissueInfosVector[15].color = Color(1.000000f, 0.705882f, 0.000000f);
	tissueInfosVector[16].name = "Diaphragm";
	tissueInfosVector[16].color = Color(0.745000f, 0.188000f, 0.286000f);
	tissueInfosVector[17].name = "Ear_cartilage";
	tissueInfosVector[17].color = Color(0.627000f, 0.988000f, 0.969000f);
	tissueInfosVector[18].name = "Ear_skin";
	tissueInfosVector[18].color = Color(0.423500f, 0.611800f, 0.603900f);
	tissueInfosVector[19].name = "Epididymis";
	tissueInfosVector[19].color = Color(0.000000f, 0.359000f, 1.000000f);
	tissueInfosVector[20].name = "Esophagus";
	tissueInfosVector[20].color = Color(1.000000f, 0.585000f, 0.000000f);
	tissueInfosVector[21].name = "Esophagus_lumen";
	tissueInfosVector[21].color = Color(1.000000f, 0.789000f, 0.635000f);
	tissueInfosVector[22].name = "Eye_lens";
	tissueInfosVector[22].color = Color(0.007800f, 0.658800f, 0.996100f);
	tissueInfosVector[23].name = "Eye_vitreous_humor";
	tissueInfosVector[23].color = Color(0.331000f, 0.746000f, 0.937000f);
	tissueInfosVector[24].name = "Fat";
	tissueInfosVector[24].color = Color(0.984314f, 0.980392f, 0.215686f);
	tissueInfosVector[25].name = "Gallbladder";
	tissueInfosVector[25].color = Color(0.258800f, 0.972500f, 0.274500f);
	tissueInfosVector[26].name = "Heart_lumen";
	tissueInfosVector[26].color = Color(1.000000f, 0.000000f, 0.000000f);
	tissueInfosVector[27].name = "Heart_muscle";
	tissueInfosVector[27].color = Color(1.000000f, 0.000000f, 0.239000f);
	tissueInfosVector[28].name = "Hippocampus";
	tissueInfosVector[28].color = Color(0.915000f, 0.188000f, 1.000000f);
	tissueInfosVector[29].name = "Hypophysis";
	tissueInfosVector[29].color = Color(1.000000f, 0.000000f, 0.796000f);
	tissueInfosVector[30].name = "Hypothalamus";
	tissueInfosVector[30].color = Color(0.563000f, 0.239000f, 0.754000f);
	tissueInfosVector[31].name = "Intervertebral_disc";
	tissueInfosVector[31].color = Color(0.627500f, 0.988200f, 0.968600f);
	tissueInfosVector[32].name = "Kidney_cortex";
	tissueInfosVector[32].color = Color(0.000000f, 0.754000f, 0.200000f);
	tissueInfosVector[33].name = "Kidney_medulla";
	tissueInfosVector[33].color = Color(0.507000f, 1.000000f, 0.479000f);
	tissueInfosVector[34].name = "Large_intestine";
	tissueInfosVector[34].color = Color(1.000000f, 0.303000f, 0.176000f);
	tissueInfosVector[35].name = "Large_intestine_lumen";
	tissueInfosVector[35].color = Color(0.817000f, 0.556000f, 0.570000f);
	tissueInfosVector[36].name = "Larynx";
	tissueInfosVector[36].color = Color(0.937000f, 0.561000f, 0.950000f);
	tissueInfosVector[37].name = "Liver";
	tissueInfosVector[37].color = Color(0.478400f, 0.262700f, 0.141200f);
	tissueInfosVector[38].name = "Lung";
	tissueInfosVector[38].color = Color(0.225000f, 0.676000f, 1.000000f);
	tissueInfosVector[39].name = "Mandible";
	tissueInfosVector[39].color = Color(0.929412f, 0.839216f, 0.584314f);
	tissueInfosVector[40].name = "Marrow_red";
	tissueInfosVector[40].color = Color(0.937300f, 0.639200f, 0.498000f);
	tissueInfosVector[41].name = "Marrow_white";
	tissueInfosVector[41].color = Color(0.921600f, 0.788200f, 0.486300f);
	tissueInfosVector[42].name = "Meniscus";
	tissueInfosVector[42].color = Color(0.577000f, 0.338000f, 0.754000f);
	tissueInfosVector[43].name = "Midbrain";
	tissueInfosVector[43].color = Color(0.490200f, 0.682400f, 0.509800f);
	tissueInfosVector[44].name = "Muscle";
	tissueInfosVector[44].color = Color(0.745098f, 0.188235f, 0.286275f);
	tissueInfosVector[45].name = "Nail";
	tissueInfosVector[45].color = Color(0.873000f, 0.887000f, 0.880000f);
	tissueInfosVector[46].name = "Mucosa";
	tissueInfosVector[46].color = Color(1.000000f, 0.631373f, 0.745098f);
	tissueInfosVector[47].name = "Nerve";
	tissueInfosVector[47].color = Color(0.000000f, 0.754000f, 0.479000f);
	tissueInfosVector[48].name = "Ovary";
	tissueInfosVector[48].color = Color(0.718000f, 0.000000f, 1.000000f);
	tissueInfosVector[49].name = "Pancreas";
	tissueInfosVector[49].color = Color(0.506000f, 0.259000f, 0.808000f);
	tissueInfosVector[50].name = "Patella";
	tissueInfosVector[50].color = Color(0.929412f, 0.839216f, 0.584314f);
	tissueInfosVector[51].name = "Penis";
	tissueInfosVector[51].color = Color(0.000000f, 0.000000f, 1.000000f);
	tissueInfosVector[52].name = "Pharynx";
	tissueInfosVector[52].color = Color(0.368600f, 0.474500f, 0.635300f);
	tissueInfosVector[53].name = "Prostate";
	tissueInfosVector[53].color = Color(0.190000f, 0.190000f, 1.000000f);
	tissueInfosVector[54].name = "Scrotum";
	tissueInfosVector[54].color = Color(0.366000f, 0.549000f, 1.000000f);
	tissueInfosVector[55].name = "Skin";
	tissueInfosVector[55].color = Color(0.746000f, 0.613000f, 0.472000f);
	tissueInfosVector[56].name = "Skull";
	tissueInfosVector[56].color = Color(0.929412f, 0.839216f, 0.584314f);
	tissueInfosVector[57].name = "Small_intestine";
	tissueInfosVector[57].color = Color(1.000000f, 0.775000f, 0.690000f);
	tissueInfosVector[58].name = "Small_intestine_lumen";
	tissueInfosVector[58].color = Color(1.000000f, 0.474500f, 0.635300f);
	tissueInfosVector[59].name = "Spinal_cord";
	tissueInfosVector[59].color = Color(0.000000f, 0.732000f, 0.662000f);
	tissueInfosVector[60].name = "Spleen";
	tissueInfosVector[60].color = Color(0.682400f, 0.964700f, 0.788200f);
	tissueInfosVector[61].name = "Stomach";
	tissueInfosVector[61].color = Color(1.000000f, 0.500000f, 0.000000f);
	tissueInfosVector[62].name = "Stomach_lumen";
	tissueInfosVector[62].color = Color(1.000000f, 0.738000f, 0.503000f);
	tissueInfosVector[63].name = "SAT";
	tissueInfosVector[63].color = Color(1.000000f, 0.796079f, 0.341176f);
	tissueInfosVector[64].name = "Teeth";
	tissueInfosVector[64].color = Color(0.976471f, 0.960784f, 0.905882f);
	tissueInfosVector[65].name = "Tendon_Ligament";
	tissueInfosVector[65].color = Color(0.945098f, 0.960784f, 0.972549f);
	tissueInfosVector[66].name = "Testis";
	tissueInfosVector[66].color = Color(0.000000f, 0.606000f, 1.000000f);
	tissueInfosVector[67].name = "Thalamus";
	tissueInfosVector[67].color = Color(0.000000f, 0.415000f, 0.549000f);
	tissueInfosVector[68].name = "Thymus";
	tissueInfosVector[68].color = Color(0.439200f, 0.733300f, 0.549000f);
	tissueInfosVector[69].name = "Thyroid_gland";
	tissueInfosVector[69].color = Color(0.321600f, 0.023500f, 0.298000f);
	tissueInfosVector[70].name = "Tongue";
	tissueInfosVector[70].color = Color(0.800000f, 0.400000f, 0.400000f);
	tissueInfosVector[71].name = "Trachea";
	tissueInfosVector[71].color = Color(0.183000f, 1.000000f, 1.000000f);
	tissueInfosVector[72].name = "Trachea_lumen";
	tissueInfosVector[72].color = Color(0.613000f, 1.000000f, 1.000000f);
	tissueInfosVector[73].name = "Ureter_Urethra";
	tissueInfosVector[73].color = Color(0.376500f, 0.607800f, 0.007800f);
	tissueInfosVector[74].name = "Uterus";
	tissueInfosVector[74].color = Color(0.894000f, 0.529000f, 1.000000f);
	tissueInfosVector[75].name = "Vagina";
	tissueInfosVector[75].color = Color(0.608000f, 0.529000f, 1.000000f);
	tissueInfosVector[76].name = "Vein";
	tissueInfosVector[76].color = Color(0.000000f, 0.329000f, 1.000000f);
	tissueInfosVector[77].name = "Vertebrae";
	tissueInfosVector[77].color = Color(0.929412f, 0.839216f, 0.584314f);
	tissueInfosVector[78].name = "Pinealbody";
	tissueInfosVector[78].color = Color(1.000000f, 0.000000f, 0.000000f);
	tissueInfosVector[79].name = "Pons";
	tissueInfosVector[79].color = Color(0.000000f, 0.710000f, 0.700000f);
	tissueInfosVector[80].name = "Medulla_oblongata";
	tissueInfosVector[80].color = Color(0.370000f, 0.670000f, 0.920000f);
	tissueInfosVector[81].name = "Cornea";
	tissueInfosVector[81].color = Color(0.686275f, 0.000000f, 1.000000f);
	tissueInfosVector[82].name = "Eye_Sclera";
	tissueInfosVector[82].color = Color(1.000000f, 0.000000f, 0.780392f);

	CreateTissueTypeMap();
}

FILE* TissueInfos::SaveTissues(FILE* fp, unsigned short version)
{
	tissues_size_t tissuecount = GetTissueCount();
	fwrite(&tissuecount, 1, sizeof(tissues_size_t), fp);

	if (version >= 5)
	{
		float id = 1.2345f;
		fwrite(&id, 1, sizeof(float), fp);
		fwrite(&version, 1, sizeof(unsigned short), fp);
	}
	fwrite(&(tissueInfosVector[0].color[0]), 1, sizeof(float), fp);
	fwrite(&(tissueInfosVector[0].color[1]), 1, sizeof(float), fp);
	fwrite(&(tissueInfosVector[0].color[2]), 1, sizeof(float), fp);
	if (version >= 5)
	{
		fwrite(&(tissueInfosVector[0].opac), 1, sizeof(float), fp);
	}

	TissueInfosVecType::iterator vecIt;
	for (vecIt = tissueInfosVector.begin() + 1;
			 vecIt != tissueInfosVector.end(); ++vecIt)
	{
		fwrite(&(vecIt->color[0]), 1, sizeof(float), fp);
		fwrite(&(vecIt->color[1]), 1, sizeof(float), fp);
		fwrite(&(vecIt->color[2]), 1, sizeof(float), fp);
		if (version >= 5)
		{
			fwrite(&(vecIt->opac), 1, sizeof(float), fp);
		}
		int size = static_cast<int>(vecIt->name.length());
		fwrite(&size, 1, sizeof(int), fp);
		fwrite(vecIt->name.c_str(), 1, sizeof(char) * size, fp);
	}

	return fp;
}

namespace {
std::vector<TissueHierarchyItem*> GetLeaves(TissueHierarchyItem* hiearchy)
{
	std::vector<TissueHierarchyItem*> leaves;
	for (auto item : *hiearchy->GetChildren())
	{
		if (item->GetIsFolder())
		{
			auto item_leaves = GetLeaves(item);
			leaves.insert(leaves.end(), item_leaves.begin(), item_leaves.end());
		}
		else
		{
			leaves.insert(leaves.end(), item);
		}
	}
	return leaves;
}

std::map<std::string, std::string>
		BuildHiearchyMap(TissueHierarchyItem* hiearchy)
{
	std::map<std::string, std::string> result;

	if (hiearchy)
	{
		auto leaves = GetLeaves(hiearchy);
		for (auto item : leaves)
		{
			std::vector<std::string> parents;
			auto current = item->GetParent();
			while (current && current != hiearchy)
			{
				parents.push_back(current->GetName().toLocal8Bit().constData());
				current = current->GetParent();
			}
			std::string SEP = "/";
			std::string path = boost::algorithm::join(parents, SEP);
			std::string tissue = item->GetName().toLocal8Bit().constData();
			result.insert(std::make_pair(tissue, path));
		}
	}

	return result;
}
} // namespace

bool TissueInfos::SaveTissuesHDF(const char* filename,
		TissueHierarchyItem* hiearchy, bool naked,
		unsigned short version)
{
	ScopedTimer timer("Write Tissue List");

	namespace fs = boost::filesystem;

	int compression = 1;

	fs::path qFileName(filename);
	std::string basename = (qFileName.parent_path() / qFileName.stem()).string();
	std::string suffix = qFileName.extension().string();

	// save working directory
	auto oldcwd = fs::current_path();

	// enter the xmf file folder so relative names for hdf5 files work
	fs::current_path(qFileName.parent_path());

	HDF5Writer writer;
	writer.loud = 0;
	std::string fname;
	if (naked)
		fname = basename + suffix;
	else
		fname = basename + ".h5";

	if (!writer.open(fname, "append"))
	{
		ISEG_ERROR("opening " << fname);
	}
	writer.compression = compression;

	auto hiearchy_map = BuildHiearchyMap(hiearchy);

	if (!writer.createGroup(std::string("Tissues")))
	{
		ISEG_ERROR_MSG("creating tissues section");
	}

	float rgbo[4];
	int index1[1];
	std::vector<HDF5Writer::size_type> dim1;
	dim1.resize(1);
	dim1[0] = 4;
	std::vector<HDF5Writer::size_type> dim2;
	dim2.resize(1);
	dim2[0] = 1;

	index1[0] = (int)version;
	if (!writer.write(index1, dim2, std::string("/Tissues/version")))
	{
		ISEG_ERROR_MSG("writing version");
	}

	rgbo[0] = tissueInfosVector[0].color[0];
	rgbo[1] = tissueInfosVector[0].color[1];
	rgbo[2] = tissueInfosVector[0].color[2];
	rgbo[3] = tissueInfosVector[0].opac;
	if (!writer.write(rgbo, dim1, std::string("/Tissues/bkg_rgbo")))
	{
		ISEG_ERROR_MSG("writing rgbo");
	}

	int counter = 1;
	for (auto vecIt = tissueInfosVector.begin() + 1; vecIt != tissueInfosVector.end(); ++vecIt)
	{
		std::string tissuename1 = vecIt->name;
		boost::replace_all(tissuename1, "\\", "_");
		boost::replace_all(tissuename1, "/", "_");
		std::string tissuename = tissuename1; //BL TODO tissuename1.toLocal8Bit().constData();

		std::string groupname = std::string("/Tissues/") + tissuename;
		writer.createGroup(groupname);
		rgbo[0] = vecIt->color[0];
		rgbo[1] = vecIt->color[1];
		rgbo[2] = vecIt->color[2];
		rgbo[3] = vecIt->opac;

		std::string path = hiearchy_map[tissuename];
		if (!writer.write_attribute(path, groupname + "/path"))
		{
			ISEG_ERROR_MSG("writing path");
		}
		if (!writer.write(rgbo, dim1, groupname + "/rgbo"))
		{
			ISEG_ERROR_MSG("writing rgbo");
		}
		index1[0] = counter++;

		if (!writer.write(index1, dim2, groupname + "/index"))
		{
			ISEG_ERROR_MSG("writing index");
		}
	}

	writer.close();

	return true;
}

FILE* TissueInfos::SaveTissueLocks(FILE* fp)
{
	TissueInfosVecType::iterator vecIt;
	for (vecIt = tissueInfosVector.begin() + 1;
			 vecIt != tissueInfosVector.end(); ++vecIt)
	{
		unsigned char dummy = (unsigned char)vecIt->locked;
		fwrite(&(dummy), 1, sizeof(unsigned char), fp);
	}

	return fp;
}

FILE* TissueInfos::LoadTissueLocks(FILE* fp)
{
	unsigned char dummy;
	tissueInfosVector[0].locked = false;
	TissueInfosVecType::iterator vecIt;
	for (vecIt = tissueInfosVector.begin() + 1;
			 vecIt != tissueInfosVector.end(); ++vecIt)
	{
		fread(&(dummy), sizeof(unsigned char), 1, fp);
		vecIt->locked = (bool)dummy;
	}

	return fp;
}

FILE* TissueInfos::LoadTissues(FILE* fp, int tissuesVersion)
{
	char name[100];
	tissues_size_t tissuecount;
	if (tissuesVersion > 0)
	{
		fread(&tissuecount, sizeof(tissues_size_t), 1, fp);
	}
	else
	{
		unsigned char ucharBuffer;
		fread(&ucharBuffer, sizeof(unsigned char), 1, fp);
		tissuecount = (tissues_size_t)ucharBuffer;
	}

	tissueInfosVector.clear();
	tissueInfosVector.resize(tissuecount + 1);

	float id;
	fread(&id, sizeof(float), 1, fp);
	unsigned short opacVersion = 0;
	if (id == 1.2345f)
	{
		fread(&opacVersion, sizeof(unsigned short), 1, fp);
		fread(&(tissueInfosVector[0].color[0]), sizeof(float), 1, fp);
	}
	else
	{
		tissueInfosVector[0].color[0] = id;
		tissueInfosVector[0].opac = 0.5f;
	}
	fread(&(tissueInfosVector[0].color[1]), sizeof(float), 1, fp);
	fread(&(tissueInfosVector[0].color[2]), sizeof(float), 1, fp);
	if (opacVersion >= 5)
	{
		fread(&(tissueInfosVector[0].opac), sizeof(float), 1, fp);
	}

	TissueInfosVecType::iterator vecIt;
	for (vecIt = tissueInfosVector.begin() + 1;
			 vecIt != tissueInfosVector.end(); ++vecIt)
	{
		vecIt->locked = false;
		fread(&(vecIt->color[0]), sizeof(float), 1, fp);
		fread(&(vecIt->color[1]), sizeof(float), 1, fp);
		fread(&(vecIt->color[2]), sizeof(float), 1, fp);
		if (opacVersion >= 5)
		{
			fread(&(vecIt->opac), sizeof(float), 1, fp);
		}
		else
		{
			vecIt->opac = 0.5f;
		}
		int size;
		fread(&size, sizeof(int), 1, fp);
		fread(name, sizeof(char) * size, 1, fp);
		if (size > 99)
			throw std::runtime_error("ERROR: in tissue file: setting.bin");
		name[size] = '\0';
		std::string s(name);
		vecIt->name = s;
	}

	CreateTissueTypeMap();

	return fp;
}

bool TissueInfos::LoadTissuesHDF(const char* filename, int tissuesVersion)
{
	ScopedTimer timer("Read Tissue List");

	HDF5Reader reader;
	if (!reader.open(filename))
	{
		ISEG_ERROR("opening " << filename);
		return false;
	}

	bool ok = true;
	std::vector<std::string> tissues = reader.getGroupInfo("/Tissues");

	tissues_size_t tissuecount = static_cast<tissues_size_t>(tissues.size() - 2);
	tissueInfosVector.clear();
	if (tissuecount == 0)
	{
		tissuecount = 1;
		tissueInfosVector.resize(2);
		tissueInfosVector[1].locked = false;
		tissueInfosVector[1].color[0] = 1.0f;
		tissueInfosVector[1].color[1] = 0;
		tissueInfosVector[1].color[2] = 0;
		tissueInfosVector[1].opac = 0.5;
		tissueInfosVector[1].name = std::string("Tissue1");
	}
	else
	{
		tissueInfosVector.resize(tissuecount + 1);
	}

	std::vector<float> rgbo;
	rgbo.resize(4);
	int index;
	for (std::vector<std::string>::iterator it = tissues.begin();
			 it != tissues.end(); it++)
	{
		if (std::string("version").compare(std::string(it->c_str())) == 0) {}
		else if (std::string("bkg_rgbo").compare(std::string(it->c_str())) == 0)
		{
			ok = ok && (reader.read(rgbo, "/Tissues/bkg_rgbo") != 0);
			tissueInfosVector[0].color[0] = rgbo[0];
			tissueInfosVector[0].color[1] = rgbo[1];
			tissueInfosVector[0].color[2] = rgbo[2];
			tissueInfosVector[0].opac = rgbo[3];
		}
		else
		{
			ok = ok && (reader.read(&index, std::string("/Tissues/") + *it + std::string("/index")) != 0);
			ok = ok && (reader.read(rgbo, std::string("/Tissues/") + *it + std::string("/rgbo")) != 0);
			tissueInfosVector[index].locked = false;
			tissueInfosVector[index].color[0] = rgbo[0];
			tissueInfosVector[index].color[1] = rgbo[1];
			tissueInfosVector[index].color[2] = rgbo[2];
			tissueInfosVector[index].opac = rgbo[3];
			tissueInfosVector[index].name = std::string(it->c_str());
		}
	}

	CreateTissueTypeMap();

	reader.close();

	return ok;
}

bool TissueInfos::SaveTissuesReadable(const char* filename,
		unsigned short version)
{
	FILE* fp;
	if ((fp = fopen(filename, "w")) == nullptr)
	{
		return false;
	}
	else
	{
		if (version >= 5)
		{
			fprintf(fp, "V%u\n", (unsigned)version);
		}
		tissues_size_t tissuecount = GetTissueCount();
		fprintf(fp, "N%u\n", tissuecount);

		TissueInfosVecType::iterator vecIt;
		if (version < 5)
		{
			for (vecIt = tissueInfosVector.begin() + 1;
					 vecIt != tissueInfosVector.end(); ++vecIt)
			{
				fprintf(fp, "C%f %f %f %s\n", vecIt->color[0], vecIt->color[1],
						vecIt->color[2], vecIt->name.c_str());
			}
		}
		else
		{
			for (vecIt = tissueInfosVector.begin() + 1;
					 vecIt != tissueInfosVector.end(); ++vecIt)
			{
				fprintf(fp, "C%f %f %f %f %s\n", vecIt->color[0],
						vecIt->color[1], vecIt->color[2], vecIt->opac,
						vecIt->name.c_str());
			}
		}

		fclose(fp);
		return true;
	}
}

bool TissueInfos::LoadTissuesReadable(const char* filename,
		SlicesHandler* handler3D,
		tissues_size_t& removeTissuesRange)
{
	// To replace the old tissue list, handler3D->remove_tissues(removeTissuesRange) has to be called afterwards!

	char name[100];
	FILE* fp;
	if ((fp = fopen(filename, "r")) == nullptr)
	{
		InitTissues();
		return false;
	}
	else
	{
		unsigned short version = 0;
		unsigned int tc;

		// Get version number
		if (fscanf(fp, "V%u\n", &tc) != 1)
		{
			fclose(fp);
			if ((fp = fopen(filename, "r")) == nullptr)
			{
				InitTissues();
				return false;
			}
		}
		else
		{
			version = tc;
		}

		// Get number of labels
		bool isFreeSurfer = false;
		int r, g, b, a, res, currLabel;
		if (fscanf(fp, "N%u\n", &tc) != 1)
		{
			fclose(fp);
			if ((fp = fopen(filename, "r")) == nullptr)
			{
				InitTissues();
				return false;
			}

			// Detect FreeSurfer color table format by searching for Unknown label
			currLabel = res = 0;
			r = g = b = a = -1;
			while (res != EOF && res != 6)
			{
				res = fscanf(fp, "%d\t%s\t%d\t%d\t%d\t%d\n", &currLabel, name,
						&r, &g, &b, &a);
				if (currLabel == r == g == b == a == 0 &&
						std::string("Unknown").compare(std::string(name)) == 0)
				{
					isFreeSurfer = true;
				}
			}
			if (!isFreeSurfer)
			{
				// Unknown format
				fclose(fp);
				InitTissues();
				return false;
			}

			// Get largest label index
			tissues_size_t labelMax = 0;
			while ((res = fscanf(fp, "%d\t%s\t%d\t%d\t%d\t%d\n", &currLabel,
									name, &r, &g, &b, &a)) != EOF)
			{
				if (res == 6)
				{
					labelMax = std::max(int(labelMax), currLabel);
				}
			}
			tc = labelMax;

			fclose(fp);
			if ((fp = fopen(filename, "r")) == nullptr)
			{
				InitTissues();
				return false;
			}
		}

		std::set<tissues_size_t> missingTissues;
		for (tissues_size_t type = 0; type <= TissueInfos::GetTissueCount();
				 ++type)
		{
			missingTissues.insert(type);
		}

		tissues_size_t tc1 = (tissues_size_t)tc;
		if (tc > TISSUES_SIZE_MAX)
			tc1 = TISSUES_SIZE_MAX;

		TissueInfosVecType newTissueInfosVec;
		if (isFreeSurfer)
		{
			// Read first tissue from file
			currLabel = res = 0;
			while (res != EOF && (res != 6 || currLabel == 0))
			{
				res = fscanf(fp, "%d\t%s\t%d\t%d\t%d\t%d\n", &currLabel, name,
						&r, &g, &b, &a);
			}

			tissues_size_t dummyIdx = 1;
			for (tissues_size_t newTypeIdx = 0; newTypeIdx < tc1; ++newTypeIdx)
			{
				TissueInfo newTissueInfo;
				newTissueInfo.locked = false;
				if (currLabel == newTypeIdx + 1)
				{
					newTissueInfo.color[0] = r / 255.0f;
					newTissueInfo.color[1] = g / 255.0f;
					newTissueInfo.color[2] = b / 255.0f;
					newTissueInfo.opac = a / 255.0f;
					newTissueInfo.name = std::string(name);

					// Read next tissue from file
					res = 0;
					while (res != EOF && res != 6)
					{
						res = fscanf(fp, "%d\t%s\t%d\t%d\t%d\t%d\n", &currLabel,
								name, &r, &g, &b, &a);
					}
				}
				else
				{
					// Dummy tissue
					newTissueInfo.color[0] = float(rand()) / RAND_MAX;
					newTissueInfo.color[1] = float(rand()) / RAND_MAX;
					newTissueInfo.color[2] = float(rand()) / RAND_MAX;
					newTissueInfo.opac = 0.5f;
					newTissueInfo.name = (boost::format("DummyTissue%1") % dummyIdx++).str();
				}
				newTissueInfosVec.push_back(newTissueInfo);

				// Check whether the tissue existed before
				tissues_size_t oldType = GetTissueType(newTissueInfo.name);
				if (oldType > 0 &&
						missingTissues.find(oldType) != missingTissues.end())
				{
					missingTissues.erase(missingTissues.find(oldType));
				}
			}
		}
		else if (version < 5)
		{
			for (tissues_size_t newTypeIdx = 0; newTypeIdx < tc1; ++newTypeIdx)
			{
				TissueInfo newTissueInfo;
				newTissueInfo.locked = false;
				if (fscanf(fp, "C%f %f %f %s\n", &(newTissueInfo.color[0]),
								&(newTissueInfo.color[1]), &(newTissueInfo.color[2]),
								name) != 4)
				{
					fclose(fp);
					InitTissues();
					return false;
				}
				newTissueInfo.opac = 0.5f;
				newTissueInfo.name = std::string(name);
				newTissueInfosVec.push_back(newTissueInfo);

				// Check whether the tissue existed before
				tissues_size_t oldType = GetTissueType(newTissueInfo.name);
				if (oldType > 0 &&
						missingTissues.find(oldType) != missingTissues.end())
				{
					missingTissues.erase(missingTissues.find(oldType));
				}
			}
		}
		else
		{
			for (tissues_size_t newTypeIdx = 0; newTypeIdx < tc1; ++newTypeIdx)
			{
				TissueInfo newTissueInfo;
				newTissueInfo.locked = false;
				if (fscanf(fp, "C%f %f %f %f %s\n", &(newTissueInfo.color[0]),
								&(newTissueInfo.color[1]), &(newTissueInfo.color[2]),
								&(newTissueInfo.opac), name) != 5)
				{
					fclose(fp);
					InitTissues();
					return false;
				}
				newTissueInfo.name = std::string(name);
				newTissueInfosVec.push_back(newTissueInfo);

				// Check whether the tissue existed before
				tissues_size_t oldType = GetTissueType(newTissueInfo.name);
				if (oldType > 0 &&
						missingTissues.find(oldType) != missingTissues.end())
				{
					missingTissues.erase(missingTissues.find(oldType));
				}
			}
		}

		// Prepend existing tissues (including background) which the input file did not contain
		removeTissuesRange = static_cast<tissues_size_t>(missingTissues.size() - 1);
		for (std::set<tissues_size_t>::reverse_iterator riter = missingTissues.rbegin(); riter != missingTissues.rend(); ++riter)
		{
			newTissueInfosVec.insert(newTissueInfosVec.begin(),
					tissueInfosVector[*riter]);
		}

		// Permute tissue indices according to ordering in input file
		std::vector<tissues_size_t> idxMap(tissueInfosVector.size());
		for (tissues_size_t oldIdx = 0; oldIdx < tissueInfosVector.size(); ++oldIdx)
		{
			idxMap[oldIdx] = oldIdx;
		}
		bool permute = false;
		for (tissues_size_t newIdx = 0; newIdx < newTissueInfosVec.size(); ++newIdx)
		{
			tissues_size_t oldIdx = GetTissueType(newTissueInfosVec[newIdx].name);
			if (oldIdx > 0 && oldIdx != newIdx)
			{
				idxMap[oldIdx] = newIdx;
				permute = true;
			}
		}
		if (permute)
		{
			handler3D->map_tissue_indices(idxMap);
		}

		// Assign new infos vector
		tissueInfosVector = newTissueInfosVec;

		fclose(fp);
		CreateTissueTypeMap();
		return true;
	}
}

bool TissueInfos::SaveDefaultTissueList(const char* filename)
{
	FILE* fp;
	fp = fopen(filename, "w");
	if (fp == nullptr)
	{
		return false;
	}

	TissueInfosVecType::iterator vecIt;
	for (vecIt = tissueInfosVector.begin() + 1;
			 vecIt != tissueInfosVector.end(); ++vecIt)
	{
		std::string tissuename1 = vecIt->name;
		boost::replace_all(tissuename1, " ", "_");
		fprintf(fp, "%s %f %f %f %f\n", tissuename1.c_str(), vecIt->color[0],
				vecIt->color[1], vecIt->color[2], vecIt->opac);
	}
	fclose(fp);
	return true;
}

bool TissueInfos::LoadDefaultTissueList(const char* filename)
{
	FILE* fp;
	fp = fopen(filename, "r");
	if (fp == nullptr)
	{
		InitTissues();
	}
	else
	{
		Color rgb;
		float alpha;
		char name1[1000];

		tissueInfosVector.clear();
		tissueInfosVector.resize(1); // Background
		while (fscanf(fp, "%s %f %f %f %f", name1, &rgb.r, &rgb.g, &rgb.b, &alpha) == 5)
		{
			TissueInfo newTissueInfo;
			newTissueInfo.locked = false;
			newTissueInfo.color = rgb;
			newTissueInfo.opac = alpha;
			newTissueInfo.name = name1;
			tissueInfosVector.push_back(newTissueInfo);
		}
		fclose(fp);
	}

	CreateTissueTypeMap();
	return true;
}

tissues_size_t TissueInfos::GetTissueCount()
{
	return static_cast<tissues_size_t>(tissueInfosVector.size()) - 1; // Tissue index 0 is the background
}

void TissueInfos::AddTissue(TissueInfo& tissue)
{
	tissueInfosVector.push_back(tissue);
	tissueTypeMap.insert(TissueTypeMapEntryType(str_tolower(tissue.name), GetTissueCount()));
}

void TissueInfos::RemoveTissue(tissues_size_t tissuetype)
{
	// tissueTypeMap.erase(tissueInfosVector[tissuetype].name.toLower());
	tissueInfosVector.erase(tissueInfosVector.begin() + tissuetype);
	CreateTissueTypeMap();
}

void TissueInfos::RemoveTissues(const std::set<tissues_size_t>& tissuetypes)
{
	// Remove in descending order
	for (auto riter = tissuetypes.rbegin(); riter != tissuetypes.rend(); ++riter)
	{
		// tissueTypeMap.erase(tissueInfosVector[*riter].name.toLower());
		tissueInfosVector.erase(tissueInfosVector.begin() + *riter);
	}
	CreateTissueTypeMap();
}

void TissueInfos::RemoveAllTissues()
{
	tissueTypeMap.clear();
	tissueInfosVector.clear();
	tissueInfosVector.resize(1); // Background
}

void TissueInfos::CreateTissueTypeMap()
{
	tissueTypeMap.clear();
	for (tissues_size_t type = 1; type <= GetTissueCount(); ++type)
	{
		tissueTypeMap.insert(TissueTypeMapEntryType(str_tolower(tissueInfosVector[type].name), type));
	}
}

namespace {
bool is_valid(tissues_size_t i)
{
	return (i <= TissueInfos::GetTissueCount());
}
std::set<tissues_size_t> _selection;
} // namespace

std::set<iseg::tissues_size_t> iseg::TissueInfos::GetSelectedTissues()
{
	std::set<tissues_size_t> r;
	std::copy_if(_selection.begin(), _selection.end(), std::inserter(r, r.end()), is_valid);
	return r;
}

void iseg::TissueInfos::SetSelectedTissues(const std::set<tissues_size_t>& sel)
{
	ISEG_INFO("Selected tissues " << sel.size());
	if (std::all_of(sel.begin(), sel.end(), is_valid))
	{
		_selection = sel;
	}
}

TissueInfos::TissueInfosVecType TissueInfos::tissueInfosVector;
TissueInfos::TissueTypeMapType TissueInfos::tissueTypeMap;
