/*MIT License

Copyright (c) 2016 MTA SZTAKI

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#ifndef APE_EVENT_H
#define APE_EVENT_H

#include <string>

namespace Ape
{
	struct Event
	{
	public:
		enum Group : unsigned int
		{
			NODE,
			LIGHT,
			CAMERA,
			GEOMETRY_FILE,
			GEOMETRY_INDEXEDFACESET,
			GEOMETRY_INDEXEDLINESET,
			GEOMETRY_TEXT,
			GEOMETRY_BOX,
			GEOMETRY_PLANE,
			GEOMETRY_TUBE,
			GEOMETRY_CYLINDER,
			GEOMETRY_TORUS,
			GEOMETRY_CONE,
			GEOMETRY_SPHERE,
			MATERIAL_MANUAL,
			MATERIAL_FILE,
			PASS_PBS,
			PASS_MANUAL,
			TEXTURE_MANUAL,
			BROWSER,
			EG_INVALID
		};

		enum Type : unsigned int
		{
			NODE_CREATE = (NODE << 8) + 1,
			NODE_DELETE,
			NODE_POSITION,
			NODE_ORIENTATION,
			NODE_SCALE,
			NODE_PARENTNODE,
			NODE_CHILDVISIBILITY,
			NODE_FIXEDYAW,
			LIGHT_CREATE = (LIGHT << 8) + 1,
			LIGHT_DELETE,
			LIGHT_TYPE,
			LIGHT_DIFFUSE,
			LIGHT_SPECULAR,
			LIGHT_ATTENUATION,
			LIGHT_DIRECTION,
			LIGHT_SPOTRANGE,
			LIGHT_PARENTNODE,
			GEOMETRY_FILE_CREATE = (GEOMETRY_FILE << 8) + 1,
			GEOMETRY_FILE_DELETE,
			GEOMETRY_FILE_FILENAME,
			GEOMETRY_FILE_EXPORT,
			GEOMETRY_FILE_MATERIAL,
			GEOMETRY_FILE_PARENTNODE,
			GEOMETRY_INDEXEDFACESET_CREATE = (GEOMETRY_INDEXEDFACESET << 8) + 1,
			GEOMETRY_INDEXEDFACESET_PARAMETERS,
			GEOMETRY_INDEXEDFACESET_MATERIAL,
			GEOMETRY_INDEXEDFACESET_PARENTNODE,
			GEOMETRY_INDEXEDFACESET_DELETE,
			GEOMETRY_INDEXEDLINESET_CREATE = (GEOMETRY_INDEXEDLINESET << 8) + 1,
			GEOMETRY_INDEXEDLINESET_PARAMETERS,
			GEOMETRY_INDEXEDLINESET_PARENTNODE,
			GEOMETRY_INDEXEDLINESET_DELETE,
			GEOMETRY_TEXT_CREATE = (GEOMETRY_TEXT << 8) + 1,
			GEOMETRY_TEXT_DELETE,
			GEOMETRY_TEXT_VISIBLE,
			GEOMETRY_TEXT_CAPTION,
			GEOMETRY_TEXT_OFFSET,
			GEOMETRY_TEXT_PARENTNODE,
			GEOMETRY_PLANE_CREATE = (GEOMETRY_PLANE << 8) + 1,
			GEOMETRY_PLANE_PARAMETERS,
			GEOMETRY_PLANE_MATERIAL,
			GEOMETRY_PLANE_PARENTNODE,
			GEOMETRY_PLANE_DELETE,
			GEOMETRY_BOX_CREATE = (GEOMETRY_BOX << 8) + 1,
			GEOMETRY_BOX_PARAMETERS,
			GEOMETRY_BOX_MATERIAL,
			GEOMETRY_BOX_PARENTNODE,
			GEOMETRY_BOX_DELETE,
			GEOMETRY_SPHERE_CREATE = (GEOMETRY_SPHERE << 8) + 1,
			GEOMETRY_SPHERE_PARAMETERS,
			GEOMETRY_SPHERE_MATERIAL,
			GEOMETRY_SPHERE_PARENTNODE,
			GEOMETRY_SPHERE_DELETE,
			GEOMETRY_CONE_CREATE = (GEOMETRY_CONE << 8) + 1,
			GEOMETRY_CONE_PARAMETERS,
			GEOMETRY_CONE_MATERIAL,
			GEOMETRY_CONE_PARENTNODE,
			GEOMETRY_CONE_DELETE,
			GEOMETRY_TUBE_CREATE = (GEOMETRY_TUBE << 8) + 1,
			GEOMETRY_TUBE_PARAMETERS,
			GEOMETRY_TUBE_MATERIAL,
			GEOMETRY_TUBE_PARENTNODE,
			GEOMETRY_TUBE_DELETE,
			GEOMETRY_TORUS_CREATE = (GEOMETRY_TORUS << 8) + 1,
			GEOMETRY_TORUS_PARAMETERS,
			GEOMETRY_TORUS_MATERIAL,
			GEOMETRY_TORUS_PARENTNODE,
			GEOMETRY_TORUS_DELETE,
			GEOMETRY_CYLINDER_CREATE = (GEOMETRY_CYLINDER << 8) + 1,
			GEOMETRY_CYLINDER_PARAMETERS,
			GEOMETRY_CYLINDER_MATERIAL,
			GEOMETRY_CYLINDER_PARENTNODE,
			GEOMETRY_CYLINDER_DELETE,
			MATERIAL_MANUAL_CREATE = (MATERIAL_MANUAL << 8) + 1,
			MATERIAL_MANUAL_AMBIENT,
			MATERIAL_MANUAL_EMISSIVE,
			MATERIAL_MANUAL_DIFFUSE,
			MATERIAL_MANUAL_SPECULAR,
			MATERIAL_MANUAL_PASS,
			MATERIAL_MANUAL_TEXTURE,
			MATERIAL_MANUAL_DELETE,
			MATERIAL_FILE_CREATE = (MATERIAL_FILE << 8) + 1,
			MATERIAL_FILE_SETASSKYBOX,
			MATERIAL_FILE_TEXTURE,
			MATERIAL_FILE_GPUPARAMETERS,
			MATERIAL_FILE_FILENAME,
			MATERIAL_FILE_DELETE,
			TEXTURE_MANUAL_CREATE = (TEXTURE_MANUAL << 8) + 1,
			TEXTURE_MANUAL_PARAMETERS,
			TEXTURE_MANUAL_SOURCECAMERA,
			TEXTURE_MANUAL_BUFFER,
			TEXTURE_MANUAL_DELETE,
			CAMERA_CREATE = (CAMERA << 8) + 1,
			CAMERA_DELETE,
			CAMERA_FOCALLENGTH,
			CAMERA_ASPECTRATIO,
			CAMERA_FOVY,
			CAMERA_FRUSTUMOFFSET,
			CAMERA_NEARCLIP,
			CAMERA_FARCLIP,
			CAMERA_PROJECTION,
			CAMERA_POSITION,
			CAMERA_ORIENTATION,
			CAMERA_PARENTNODE,
			CAMERA_PROJECTIONTYPE,
			CAMERA_ORTHOWINDOWSIZE,
			CAMERA_WINDOW,
			PASS_PBS_CREATE = (PASS_PBS << 8) + 1,
			PASS_PBS_DELETE,
			PASS_PBS_AMBIENT,
			PASS_PBS_DIFFUSE,
			PASS_PBS_SPECULAR,
			PASS_PBS_EMISSIVE,
			PASS_PBS_SHININESS,
			PASS_PBS_ALBEDO,
			PASS_PBS_ROUGHNESS,
			PASS_PBS_LIGHTROUGHNESSOFFSET,
			PASS_PBS_F0,
			PASS_MANUAL_CREATE = (PASS_MANUAL << 8) + 1,
			PASS_MANUAL_DELETE,
			PASS_MANUAL_AMBIENT,
			PASS_MANUAL_DIFFUSE,
			PASS_MANUAL_SPECULAR,
			PASS_MANUAL_EMISSIVE,
			PASS_MANUAL_SHININESS,
			PASS_MANUAL_SCENEBLENDING,
			PASS_MANUAL_TEXTURE,
			PASS_MANUAL_GPUPARAMETERS,
			BROWSER_CREATE = (BROWSER << 8) + 1,
			BROWSER_URL,
			BROWSER_RESOLUTION,
			BROWSER_GEOMETRY,
			BROWSER_DELETE,
			ET_INVALID
		};
		
		std::string subjectName;

		Event::Type type;

		Event::Group group;

		Event()
		{
			subjectName = "NONE";
			type = Event::Type::ET_INVALID;
			group = Event::Group::EG_INVALID;
		}

		Event(std::string subjectName, Event::Type type)
		{
			this->subjectName = subjectName;
			this->type = type;
			this->group = ((Event::Group)((this->type & 0xFF00) >> 8));
		}

		bool operator ==(const Event& other) const
		{
			return (subjectName == other.subjectName && type == other.type);
		}
	};
}
#endif
