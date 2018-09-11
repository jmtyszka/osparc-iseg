/*
* Copyright (c) 2018 The Foundation for Research on Information Technologies in Society (IT'IS).
*
* This file is part of iSEG
* (see https://github.com/ITISFoundation/osparc-iseg).
*
* This software is released under the MIT License.
*  https://opensource.org/licenses/MIT
*/
#pragma once

#include "iSegData.h"

#include "ProgressInfo.h"

#include <itkCommand.h>
#include <itkProcessObject.h>

namespace iseg {

class ISEG_DATA_API ItkProgressObserver : public itk::Command
{
public:
	using Self = ItkProgressObserver;
	using Superclass = itk::Command;
	using Pointer = itk::SmartPointer<Self>;

	itkNewMacro(ItkProgressObserver);

public:
	void SetProgressInfo(ProgressInfo* p) { progress_info = p; }

	void Execute(itk::Object* caller, const itk::EventObject& event) override;
	void Execute(const itk::Object* object, const itk::EventObject& event) override;

protected:
	ItkProgressObserver() {}

	ProgressInfo* progress_info = nullptr;
};

} // namespace iseg
