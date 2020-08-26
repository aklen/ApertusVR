/*MIT License

Copyright (c) 2018 MTA SZTAKI

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

package org.apertusvr;

import androidx.annotation.Nullable;

public final class apeEvent {
    public enum Group {
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
        GEOMETRY_RAY,
        GEOMETRY_CLONE,
        MATERIAL_MANUAL,
        MATERIAL_FILE,
        TEXTURE_MANUAL,
        TEXTURE_FILE,
        TEXTURE_UNIT,
        BROWSER,
        WATER,
        SKY,
        POINT_CLOUD,
        EG_INVALID,
        PHYSICS
    }

    public enum Type {
        NODE_CREATE, //= (NODE << 8) + 1,
        NODE_DELETE,
        NODE_POSITION,
        NODE_ORIENTATION,
        NODE_SCALE,
        NODE_PARENTNODE,
        NODE_DETACH,
        NODE_CHILDVISIBILITY,
        NODE_VISIBILITY,
        NODE_FIXEDYAW,
        NODE_SHOWBOUNDINGBOX,
        NODE_HIDEBOUNDINGBOX,
        NODE_INHERITORIENTATION,
        NODE_INITIALSTATE,
        LIGHT_CREATE, //= (LIGHT << 8) + 1,
        LIGHT_DELETE,
        LIGHT_TYPE,
        LIGHT_DIFFUSE,
        LIGHT_SPECULAR,
        LIGHT_ATTENUATION,
        LIGHT_DIRECTION,
        LIGHT_SPOTRANGE,
        LIGHT_PARENTNODE,
        GEOMETRY_FILE_CREATE, //= (GEOMETRY_FILE << 8) + 1,
        GEOMETRY_FILE_DELETE,
        GEOMETRY_FILE_FILENAME,
        GEOMETRY_FILE_EXPORT,
        GEOMETRY_FILE_MERGESUBMESHES,
        GEOMETRY_FILE_MATERIAL,
        GEOMETRY_FILE_PARENTNODE,
        GEOMETRY_FILE_VISIBILITY,
        GEOMETRY_INDEXEDFACESET_CREATE, //= (GEOMETRY_INDEXEDFACESET << 8) + 1,
        GEOMETRY_INDEXEDFACESET_PARAMETERS,
        GEOMETRY_INDEXEDFACESET_MATERIAL,
        GEOMETRY_INDEXEDFACESET_PARENTNODE,
        GEOMETRY_INDEXEDFACESET_DELETE,
        GEOMETRY_INDEXEDLINESET_CREATE, //= (GEOMETRY_INDEXEDLINESET << 8) + 1,
        GEOMETRY_INDEXEDLINESET_PARAMETERS,
        GEOMETRY_INDEXEDLINESET_PARENTNODE,
        GEOMETRY_INDEXEDLINESET_DELETE,
        GEOMETRY_TEXT_CREATE, //= (GEOMETRY_TEXT << 8) + 1,
        GEOMETRY_TEXT_DELETE,
        GEOMETRY_TEXT_VISIBLE,
        GEOMETRY_TEXT_SHOWONTOP,
        GEOMETRY_TEXT_CAPTION,
        GEOMETRY_TEXT_PARENTNODE,
        GEOMETRY_PLANE_CREATE, //= (GEOMETRY_PLANE << 8) + 1,
        GEOMETRY_PLANE_PARAMETERS,
        GEOMETRY_PLANE_MATERIAL,
        GEOMETRY_PLANE_PARENTNODE,
        GEOMETRY_PLANE_DELETE,
        GEOMETRY_BOX_CREATE, //= (GEOMETRY_BOX << 8) + 1,
        GEOMETRY_BOX_PARAMETERS,
        GEOMETRY_BOX_MATERIAL,
        GEOMETRY_BOX_PARENTNODE,
        GEOMETRY_BOX_DELETE,
        GEOMETRY_SPHERE_CREATE, //= (GEOMETRY_SPHERE << 8) + 1,
        GEOMETRY_SPHERE_PARAMETERS,
        GEOMETRY_SPHERE_MATERIAL,
        GEOMETRY_SPHERE_PARENTNODE,
        GEOMETRY_SPHERE_DELETE,
        GEOMETRY_CONE_CREATE, //= (GEOMETRY_CONE << 8) + 1,
        GEOMETRY_CONE_PARAMETERS,
        GEOMETRY_CONE_MATERIAL,
        GEOMETRY_CONE_PARENTNODE,
        GEOMETRY_CONE_DELETE,
        GEOMETRY_TUBE_CREATE, //= (GEOMETRY_TUBE << 8) + 1,
        GEOMETRY_TUBE_PARAMETERS,
        GEOMETRY_TUBE_MATERIAL,
        GEOMETRY_TUBE_PARENTNODE,
        GEOMETRY_TUBE_DELETE,
        GEOMETRY_TORUS_CREATE, //= (GEOMETRY_TORUS << 8) + 1,
        GEOMETRY_TORUS_PARAMETERS,
        GEOMETRY_TORUS_MATERIAL,
        GEOMETRY_TORUS_PARENTNODE,
        GEOMETRY_TORUS_DELETE,
        GEOMETRY_CYLINDER_CREATE, //= (GEOMETRY_CYLINDER << 8) + 1,
        GEOMETRY_CYLINDER_PARAMETERS,
        GEOMETRY_CYLINDER_MATERIAL,
        GEOMETRY_CYLINDER_PARENTNODE,
        GEOMETRY_CYLINDER_DELETE,
        GEOMETRY_RAY_CREATE, //= (GEOMETRY_RAY << 8) + 1,
        GEOMETRY_RAY_INTERSECTIONENABLE,
        GEOMETRY_RAY_INTERSECTION,
        GEOMETRY_RAY_INTERSECTIONQUERY,
        GEOMETRY_RAY_PARENTNODE,
        GEOMETRY_RAY_DELETE,
        GEOMETRY_CLONE_CREATE, //= (GEOMETRY_CLONE << 8) + 1,
        GEOMETRY_CLONE_SOURCEGEOMETRY,
        GEOMETRY_CLONE_SOURCEGEOMETRYGROUP_NAME,
        GEOMETRY_CLONE_PARENTNODE,
        GEOMETRY_CLONE_DELETE,
        MATERIAL_MANUAL_CREATE, //= (MATERIAL_MANUAL << 8) + 1,
        MATERIAL_MANUAL_AMBIENT,
        MATERIAL_MANUAL_EMISSIVE,
        MATERIAL_MANUAL_DIFFUSE,
        MATERIAL_MANUAL_SPECULAR,
        MATERIAL_MANUAL_CULLINGMODE,
        MATERIAL_MANUAL_SCENEBLENDING,
        MATERIAL_MANUAL_OVERLAY,
        MATERIAL_MANUAL_TEXTURE,
        MATERIAL_MANUAL_MANUALCULLINGMODE,
        MATERIAL_MANUAL_DEPTHBIAS,
        MATERIAL_MANUAL_DEPTHWRITE,
        MATERIAL_MANUAL_DEPTHCHECK,
        MATERIAL_MANUAL_LIGHTING,
        MATERIAL_MANUAL_DELETE,
        MATERIAL_FILE_CREATE, //= (MATERIAL_FILE << 8) + 1,
        MATERIAL_FILE_SETASSKYBOX,
        MATERIAL_FILE_TEXTURE,
        MATERIAL_FILE_GPUPARAMETERS,
        MATERIAL_FILE_FILENAME,
        MATERIAL_FILE_DELETE,
        TEXTURE_MANUAL_CREATE, //= (TEXTURE_MANUAL << 8) + 1,
        TEXTURE_MANUAL_PARAMETERS,
        TEXTURE_MANUAL_SOURCECAMERA,
        TEXTURE_MANUAL_GRAPHICSAPIID,
        TEXTURE_MANUAL_BUFFER,
        TEXTURE_MANUAL_DELETE,
        TEXTURE_FILE_CREATE, //= (TEXTURE_FILE << 8) + 1,
        TEXTURE_FILE_FILENAME,
        TEXTURE_FILE_TYPE,
        TEXTURE_FILE_DELETE,
        TEXTURE_UNIT_CREATE, //= (TEXTURE_UNIT << 8) + 1,
        TEXTURE_UNIT_PARAMETERS,
        TEXTURE_UNIT_SCROLL,
        TEXTURE_UNIT_ADDRESSING,
        TEXTURE_UNIT_FILTERING,
        TEXTURE_UNIT_DELETE,
        CAMERA_CREATE, //= (CAMERA << 8) + 1,
        CAMERA_DELETE,
        CAMERA_FOCALLENGTH,
        CAMERA_ASPECTRATIO,
        CAMERA_AUTOASPECTRATIO,
        CAMERA_FOVY,
        CAMERA_FRUSTUMOFFSET,
        CAMERA_NEARCLIP,
        CAMERA_FARCLIP,
        CAMERA_PROJECTION,
        CAMERA_PARENTNODE,
        CAMERA_PROJECTIONTYPE,
        CAMERA_ORTHOWINDOWSIZE,
        CAMERA_WINDOW,
        CAMERA_VISIBILITY,
        BROWSER_CREATE, //= (BROWSER << 8) + 1,
        BROWSER_URL,
        BROWSER_RESOLUTION,
        BROWSER_GEOMETRY,
        BROWSER_OVERLAY,
        BROWSER_ZOOM,
        BROWSER_MOUSE_CLICK,
        BROWSER_MOUSE_SCROLL,
        BROWSER_MOUSE_MOVED,
        BROWSER_KEY_VALUE,
        BROWSER_FOCUS_ON_EDITABLE_FIELD,
        BROWSER_RELOAD,
        BROWSER_ELEMENT_CLICK,
        BROWSER_HOVER_IN,
        BROWSER_HOVER_OUT,
        BROWSER_MESSAGE,
        BROWSER_DELETE,
        WATER_CREATE, //= (WATER << 8) + 1,
        WATER_SKY,
        WATER_CAMERAS,
        WATER_DELETE,
        SKY_CREATE, //= (SKY << 8) + 1,
        SKY_TIME,
        SKY_SUNLIGHT,
        SKY_SKYLIGHT,
        SKY_SIZE,
        SKY_DELETE,
        POINT_CLOUD_CREATE, //= (POINT_CLOUD << 8) + 1,
        POINT_CLOUD_PARAMETERS,
        POINT_CLOUD_POINTS,
        POINT_CLOUD_COLORS,
        POINT_CLOUD_PARENTNODE,
        POINT_CLOUD_DELETE,
        ET_INVALID,
        RIGIDBODY_CREATE, //= (PHYSICS << 8) + 1,
        RIGIDBODY_DELETE,
        RIGIDBODY_MASS,
        RIGIDBODY_FRICTION,
        RIGIDBODY_DAMPING,
        RIGIDBODY_BOUYANCY,
        RIGIDBODY_RESTITUTION,
        RIGIDBODY_PARENTNODE,
        RIGIDBODY_SHAPE
    }

    public interface EventCallback {
        void callback(apeEvent event);
    }


    public apeEvent() {
        subjectName = "NONE";
        type = Type.ET_INVALID;
        group = Group.EG_INVALID;
    }

    public apeEvent(String subjectName, Type type, Group group) {
        this.subjectName = subjectName;
        this.type = type;
        this.group = group;
    }

    @Override
    public boolean equals(@Nullable Object obj) {
        assert obj != null;
        if(this.getClass() != obj.getClass()) {
            return false;
        } else {
            apeEvent other = (apeEvent) obj;
            return this.subjectName.equals(other.subjectName) &&
                    this.type == other.type;
        }
    }

    public String subjectName;
    public Group group;
    public Type type;
}
