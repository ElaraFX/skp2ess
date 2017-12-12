/**************************************************************************
 * Copyright (C) 2017 Rendease Co., Ltd.
 * All rights reserved.
 *
 * This program is commercial software: you must not redistribute it 
 * and/or modify it without written permission from Rendease Co., Ltd.
 *
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * End User License Agreement for more details.
 *
 * You should have received a copy of the End User License Agreement along 
 * with this program.  If not, see <http://www.rendease.com/licensing/>
 *************************************************************************/

#ifndef SKP2ESS_H
#define SKP2ESS_H

#include <string>

struct skp2ess_config
{
	std::string skp_filename;
	std::string skp_projectname;
	std::string outputpath;
	std::string outputfileprefix;
	std::string outputfiletype;
	std::string exepath;
};

extern "C" __declspec(dllexport) void skpCloudRender(skp2ess_config &sg);


#endif
