﻿
#include "stdafx.h"

static class CSampleRdkAddRdkMaterials : public CRhinoCommand
{
protected:
	virtual UUID CommandUUID() override { static const UUID uuid = { 0x43991fff, 0x967a, 0x44d8, { 0x86, 0x60, 0x2f, 0x66, 0xc7, 0x40, 0xf2, 0x12 } }; return uuid; }
	virtual const wchar_t* EnglishCommandName() override { return L"SampleRdkAddRdkMaterials"; }
	virtual CRhinoCommand::result RunCommand(const CRhinoCommandContext& context) override;
}
theSampleRdkAddRdkMaterials;

CRhinoCommand::result CSampleRdkAddRdkMaterials::RunCommand(const CRhinoCommandContext& context)
{
	auto* pDoc = context.Document();
	if (nullptr == pDoc)
		return failure;

	// Create an ON_Material.
	ON_Material mat;
	ON_Color col(255, 0, 0); // Initial color.
	mat.SetDiffuse(col);
	mat.SetName(L"Sample");

	// Create a Basic (Custom) Material using the ON_Material.
	auto* pMaterial = ::RhRdkNewBasicMaterial(mat, pDoc);
	if (nullptr == pMaterial)
		return failure;

	// Optionally set the diffuse color again.
	pMaterial->SetDiffuse(RGB(0, 255, 0));

	// Add a bitmap texture to the bitmap slot of the Basic Material.
	CRhRdkSimulatedTexture tex;
	tex.SetFilename(L"C:\\example_image.jpg", pDoc, false); // TODO: Set the full path to the file.
	auto* pBitmap = ::RhRdkNewBitmapTexture(tex, pDoc);
	if (nullptr != pBitmap)
	{
		pMaterial->SetChild(pBitmap, CS_MAT_BITMAP_TEXTURE);
		pMaterial->SetChildSlotOn(CS_MAT_BITMAP_TEXTURE, true);
	}

	// Add a wood texture to the transparency slot of the Basic Material.
	auto* pWood =	::RhRdkContentFactoriesEx().NewContentFromTypeEx(uuidWoodTextureType, pDoc);
	if (nullptr != pWood)
	{
		pMaterial->SetChild(pWood, CS_MAT_TRANSPARENCY_TEXTURE);
		pMaterial->SetChildSlotOn(CS_MAT_TRANSPARENCY_TEXTURE, true);
	}

	// This section is optional and demonstrates how to add a dib texture to the bump slot of the Basic Material.
	{{{
		const int width = 320, height = 240, depth = 32;
		CRhinoDib dib(width, height, depth);

		auto func = [](CRhinoDib::Pixel& pixel, void*)
		{
			// TODO: Fill the dib from your memory pixels.
			pixel.Set(rand()&255, rand()&255, rand()&255, 255);
			return true;
		};

		dib.ProcessPixels(func);

		// Create a dib texture from the dib and set it as a child of the Basic Material in the bump texture slot.
		auto* pTexture = ::RhRdkNewDibTexture(&dib, pDoc);
		if (nullptr != pTexture)
		{
			pMaterial->SetChild(pTexture, CS_MAT_BUMP_TEXTURE);
			pMaterial->SetChildSlotOn(CS_MAT_BUMP_TEXTURE, true);
		}
	}}}

	// Attach the Basic Material to the document.
	auto& contents = pDoc->Contents().BeginChange(RhRdkChangeContext::Program);
	contents.Attach(*pMaterial);
	contents.EndChange();

	// Set the ON_Material's material plug-in id to universal.
	mat.SetMaterialPlugInId(uuidUniversalRenderEngine);

	// Set the ON_Material's RDK material instance id to the RDK material.
	mat.SetRdkMaterialInstanceId(pMaterial->InstanceId());

	// Add the ON_Material to the material table.
	const int matIndex = pDoc->m_material_table.AddMaterial(mat);
	if (matIndex < 0)
		return failure;

	// Create a sphere object and assign the material to it.
	ON_3dmObjectAttributes attr;
	attr.m_material_index = matIndex;
	attr.SetMaterialSource(ON::material_from_object);

	const ON_Sphere sphere(ON_3dPoint::Origin, 8);
	ON_NurbsSurface ns;
	sphere.GetNurbForm(ns);
	pDoc->AddSurfaceObject(ns, &attr);

	// Do the same for a layer.
	ON_Layer layer;
	layer.m_material_index = matIndex;
	pDoc->m_layer_table.AddLayer(layer);

	pDoc->DeferredRedraw();

	return success;
}
