#include <stdio.h>
#include <string.h>

#include "graphics/al_Model.hpp"
#include "io/al_File.hpp"

using namespace al;
using namespace gfx;

#define MODEL_PARSER_BUF_LEN (256)

// @http://www.fileformat.info/format/material/
void Model :: readMTL(std::string name)
{
    FILE* file;
    std::string filename;
	char dir[PATH_MAX];
    char buf[MODEL_PARSER_BUF_LEN];
    path2dir(dir, mPath.data());
	
    printf("path %s\n", dir);
    filename = dir + name;
	
	printf("Reading material file: %s\n", filename.data());
    
    file = fopen(filename.data(), "r");
    if (!file) {
        fprintf(stderr, "readMTL() failed: can't open material file \"%s\".\n",
            filename.data());
        return;
    }
	
	mMaterials.insert(std::make_pair("default", Material()));
	Material * current = &mMaterials["default"];
	
	Color color;
	float shininess;
	int illum;
	float Tf[3];
	float dissolve;
	float sharpness;
	float optical_density;
	
    while(fscanf(file, "%s", buf) != EOF) {
		switch(buf[0]) {
			case 'n':               /* newmtl */
				fgets(buf, MODEL_PARSER_BUF_LEN, file);
				sscanf(buf, "%s %s", buf, buf);
				mMaterials.insert(std::make_pair(buf, Material()));
				printf(">>> mtl %s\n", buf);
				current = &mMaterials[buf];
				break;
			case 'N':
				switch(buf[1]) {
					case 's':
						fscanf(file, "%f", &shininess);
						/* wavefront shininess is from [0, 1000], so scale for OpenGL */
						current->shininess(shininess*128./1000.);
						break;
					case 'i':
						fscanf(file, "%f", &optical_density);
						current->opticalDensity(optical_density*128./1000.);
						break;
					default:
						/* eat up rest of line */
						fgets(buf, MODEL_PARSER_BUF_LEN, file);
						break;
					}
				break;
			
			case 'K':
				// TODO: handle 'spectral' and 'xyz' flags
				switch(buf[1]) {
					case 'd':
						fscanf(file, "%f %f %f",
							&color.r,
							&color.g,
							&color.b);
						current->diffuse(color);
						break;
					case 's':
						fscanf(file, "%f %f %f",
							&color.r,
							&color.g,
							&color.b);
						current->specular(color);
						break;
					case 'a':
						fscanf(file, "%f %f %f",
							&color.r,
							&color.g,
							&color.b);
						current->ambient(color);
						break;
					default:
						/* eat up rest of line */
						fgets(buf, MODEL_PARSER_BUF_LEN, file);
						break;
					}
				break;
			case 'm':
				switch(buf[4]) {
					case 'K':
						switch(buf[5]) {
							case 'a':
								fscanf(file, "%s", buf);
								current->ambientMap(buf);
								break;
							case 'd':
								fscanf(file, "%s", buf);
								current->diffuseMap(buf);
								break;
							case 's':
								fscanf(file, "%s", buf);
								current->specularMap(buf);
								break;
							default:
								printf("unhandled: %s\n", buf);
								/* eat up rest of line */
								fgets(buf, MODEL_PARSER_BUF_LEN, file);
								break;
							}
						break;
					case 'd':
						fscanf(file, "%s", buf);
						printf("unhandled: map_d: %s\n", buf);
						break;
					case 'a':
						fscanf(file, "%s", buf);
						printf("unhandled: map_aat: %s\n", buf);
						break;
					case 'N':
						fscanf(file, "%s", buf);
						printf("unhandled: map_Ns: %s\n", buf);
						break;
					default:
						printf("unhandled: %s\n", buf);
						/* eat up rest of line */
						fgets(buf, MODEL_PARSER_BUF_LEN, file);
						break;
					}
				break;
			case 'b':
				switch(buf[1]) {
					case 'u':
						fscanf(file, "%s", buf);
						current->bumpMap(buf);
						break;
					default:
						printf("unhandled: %s\n", buf);
						/* eat up rest of line */
						fgets(buf, MODEL_PARSER_BUF_LEN, file);
						break;
					}
				break;
			case 'i':
				// illumination model
				/*
					0	 Color on and Ambient off 
					1	 Color on and Ambient on 
					2	 Highlight on 
					3	 Reflection on and Ray trace on 
					4	 Transparency: Glass on 
					Reflection: Ray trace on 
					5	 Reflection: Fresnel on and Ray trace on 
					6	 Transparency: Refraction on 
					Reflection: Fresnel off and Ray trace on 
					7	 Transparency: Refraction on 
					Reflection: Fresnel on and Ray trace on 
					8	 Reflection on and Ray trace off 
					9	 Transparency: Glass on 
					Reflection: Ray trace off 
					10	 Casts shadows onto invisible surfaces 
				*/
				fscanf(file, "%d", &illum);
				current->illumination(illum);
				break;
			case 'T':
				// transmission filter 
				// (what colors are filtered by this object as light passes through)
				// TODO: handle 'spectral' and 'xyz' flags
				fscanf(file, "%f %f %f", &Tf[0], &Tf[1], &Tf[2]);
				printf("unhandled: Tf: %f %f %f\n", Tf[0], Tf[1], Tf[2]);
				break;
			case 'd':
				// todo: handle -halo flag
				fscanf(file, "%f", &dissolve);
				printf("unhandled: d: %f\n", dissolve);
				break;
			case 's':
				// sharpness
				fscanf(file, "%f", &sharpness);
				printf("unhandled: sharpness: %f\n", sharpness);
				break;

			default:
				printf("unhandled: %s\n", buf);
				/* eat up rest of line */
				fgets(buf, MODEL_PARSER_BUF_LEN, file);
				break;
        }
    }
    
    rewind(file);
	
	int nummaterials = mMaterials.size();
	printf("%d materials\n", nummaterials);
	
	// dump materials:
	std::map<std::string, Material>::iterator iter = mMaterials.begin();
	while (iter != mMaterials.end()) {
		printf("material %s\n", iter->first.data());
		Material& m = iter->second;
		printf("N %f\n", m.shininess());
		printf("d %f %f %f\n", m.diffuse().r, m.diffuse().g, m.diffuse().b);
		printf("s %f %f %f\n", m.specular().r, m.specular().g, m.specular().b);
		printf("a %f %f %f\n", m.ambient().r, m.ambient().g, m.ambient().b);
		printf("map a: %s s: %s d: %s\n", m.ambientMap().data(), m.specularMap().data(), m.diffuseMap().data());
		iter++;
	}
	
    fclose(file);
}

Material& Model::material(std::string name) { 
	std::map<std::string, Material>::iterator iter = mMaterials.find(name); 
	if (iter != mMaterials.end()) {
		return iter->second;
	} else {
		return mMaterials["default"];
	}
}

Model::Group& Model::group(std::string name) { 
	std::map<std::string, Group>::iterator iter = mGroups.find(name); 
	if (iter != mGroups.end()) {
		return iter->second;
	} else {
		return mGroups["default"];
	}
}

Model::Group * Model::addGroup(std::string name)
{
    Model::Group * g = &mGroups[name];
	g->name(name);
	return g;
}

struct FaceVertex {
	unsigned int index;
	unsigned int texcoord;	
	unsigned int normal;
	 
	void reset() { index = 0; texcoord = 0; normal = 0; }
	
	FaceVertex() { reset(); }
	FaceVertex(unsigned int index, unsigned int texcoord, unsigned int normal) 
	: index(index), texcoord(texcoord), normal(normal) {}
	FaceVertex(const FaceVertex& cpy) { index = cpy.index; texcoord = cpy.texcoord; normal = cpy.normal; }
};


/*
	faces can either be v, v/t, v/t/n or v//n
*/

struct Parser {
	std::vector<GraphicsData::Vertex> vertices;
	std::vector<GraphicsData::TexCoord2> texcoords;
	std::vector<GraphicsData::Normal> normals;
	
	FILE * file;
	char buf[MODEL_PARSER_BUF_LEN];
	
	Parser(FILE * file) : file(file) 
	{
		vertices.push_back(GraphicsData::Vertex());
		texcoords.push_back(GraphicsData::TexCoord2());
		normals.push_back(GraphicsData::Normal());
	}
	
	// maps face vertices (as string) to corresponding GraphicsData indices
	// this way, avoid inserting the same vertex/tex/norm combo twice, 
	// and use index buffer instead.
	std::map<std::string, int> vertexMap;
	
	void readToken() {
		if (fscanf(file, "%s", buf) == EOF)	{
			buf[0] = '\0';
		}
	}
	
	bool hasToken() {
		return buf[0] != '\0';
	}	
	
	void readLine() {
		fgets(buf, MODEL_PARSER_BUF_LEN, file);
	}           
	
	void eatLine() {
		fgets(buf, MODEL_PARSER_BUF_LEN, file);
		readToken();
	}
	
	int findVertex(std::string s) {
		std::map<std::string, int>::iterator iter = vertexMap.find(s); 
		if (iter != vertexMap.end()) {
			return iter->second;
		} else {
			// new vertex: parse it?
			return -1;
		}
	}
	
	int addV(Model::Group& g, std::string buf, int v) {
		int idx;
		std::map<std::string, int>::iterator iter = vertexMap.find(buf); 
		if (iter == vertexMap.end()) {
			idx = g.data().vertices().size();
			g.data().addVertex(vertices[v]);
			vertexMap[buf] = idx;
		} else {
			idx = iter->second;
		} 
		readToken();
		return idx;
	}
	
	int addVT(Model::Group& g, std::string buf, int v, int t) {
		int idx;
		std::map<std::string, int>::iterator iter = vertexMap.find(buf); 
		if (iter == vertexMap.end()) {
			idx = g.data().vertices().size();
			g.data().addVertex(vertices[v]);
			g.data().addTexCoord(texcoords[t]);
			vertexMap[buf] = idx;
		} else {
			idx = iter->second;
		} 
		readToken();
		return idx;
	}
	
	int addVN(Model::Group& g, std::string buf, int v, int n) {
		int idx;
		std::map<std::string, int>::iterator iter = vertexMap.find(buf); 
		if (iter == vertexMap.end()) {
			idx = g.data().vertices().size();
			g.data().addVertex(vertices[v]);
			g.data().addNormal(normals[n]);
			vertexMap[buf] = idx;
		} else {
			idx = iter->second;
		} 
		readToken();
		return idx;
	}
	
	int addVTN(Model::Group& g, std::string buf, int v, int t, int n) {
		int idx;
		std::map<std::string, int>::iterator iter = vertexMap.find(buf); 
		if (iter == vertexMap.end()) {
			idx = g.data().vertices().size();
			g.data().addVertex(vertices[v]);
			g.data().addTexCoord(texcoords[t]);
			g.data().addNormal(normals[n]);	
			vertexMap[buf] = idx;
		} else {
			idx = iter->second;
		} 
		readToken();
		return idx;
	}
	
	void addTriangle(Model::Group& g, unsigned int id0, unsigned int id1, unsigned int id2) {
		GraphicsData& data = g.data();
		
		// store this triangle in the GraphicsData indices() buffer:
		data.addIndex(id0);
		data.addIndex(id1);
		data.addIndex(id2);
		
//		Buffer<GraphicsData::Vertex>& vs = data.vertices();
//		Model::Triangle tri;
//		tri.indices[0] = id0;
//		tri.indices[1] = id1;
//		tri.indices[2] = id2;
//		// autonormal:
//		// normal<float>(tri.normal, data.vs[id0], data.vs[id1], data.vs[id2]);
//		g.mTriangles.push_back(tri);
	}
	
	std::string parseMaterial() {
		readLine();
		sscanf(buf, "%s %s", buf, buf);
		std::string mtl = buf;
		readToken();
		return mtl;
	}
	
	std::string parseMaterialLib() {
		readLine();
		sscanf(buf, "%s %s", buf, buf);
		std::string lib = buf;
		readToken();
		return lib;
	}
	
	std::string parseGroup() {
		/* eat up rest of line */
		readLine();
	#if SINGLE_STRING_GROUP_NAMES
		sscanf(buf, "%s", buf);
	#else
		buf[strlen(buf)-1] = '\0';  /* nuke '\n' */
	#endif
		std::string grp = buf;
		//printf(">> group %s\n", grp);
		readToken();
		return grp;
	}
	
	void parseVertex(FILE * file) {
		GraphicsData::Vertex vertex;
		fscanf(file, "%f %f %f", 
			&vertex[0],
			&vertex[1],
			&vertex[2]);
		vertices.push_back(vertex);
		readToken();
	}
	
	void parseTexcoord(FILE * file) {
		GraphicsData::TexCoord2 texcoord;
		fscanf(file, "%f %f", 
			&texcoord[0],
			&texcoord[1]);
		texcoords.push_back(texcoord);
		readToken();
	}
	
	void parseNormal(FILE * file) {
		GraphicsData::Normal normal;
		fscanf(file, "%f %f %f", 
			&normal[0],
			&normal[1],
			&normal[2]);
		normals.push_back(normal);
		readToken();
	}
	
	void parseFace(FILE * file, Model::Group& g) {
		FaceVertex f;
		int v, t, n;
		// indices into the group's data().vertices() buffer. 
		// vertexMap holds a reference from vertex token string to this index
		int id0, id1, id2;	
		
		std::map<std::string, int>::iterator iter;
		
		readToken();
		/* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
		if (strstr(buf, "//")) {
			sscanf(buf, "%d//%d", &v, &n);
			id0 = addVN(g, buf, v, n);
			
			sscanf(buf, "%d//%d", &v, &n);
			id1 = addVN(g, buf, v, n);
			
			sscanf(buf, "%d//%d", &v, &n);
			id2 = addVN(g, buf, v, n);

			addTriangle(g, id0, id1, id2);
			
			while(sscanf(buf, "%d//%d", &v, &n) == 2) {
				id1 = id2;
				id2 = addVN(g, buf, v, n);
				addTriangle(g, id0, id1, id2);
			}
		} else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
			id0 = addVTN(g, buf, v, t, n);
			
			sscanf(buf, "%d/%d/%d", &v, &t, &n);
			id1 = addVTN(g, buf, v, t, n);
			
			sscanf(buf, "%d/%d/%d", &v, &t, &n);
			id2 = addVTN(g, buf, v, t, n);
			
			addTriangle(g, id0, id1, id2);
			
			while(sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
				id1 = id2;
				id2 = addVTN(g, buf, v, t, n);
				addTriangle(g, id0, id1, id2);
			}
		} else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
			id0 = addVT(g, buf, v, t);
			
			fscanf(file, "%s", buf);
			sscanf(buf, "%d/%d", &v, &t);
			id1 = addVT(g, buf, v, t);
			
			sscanf(buf, "%d/%d", &v, &t);
			id2 = addVT(g, buf, v, t);
			
			addTriangle(g, id0, id1, id2);
			
			while(sscanf(buf, "%d/%d", &v, &t) == 2) {
				id1 = id2;
				id2 = addVT(g, buf, v, t);
				addTriangle(g, id0, id1, id2);
			}
		} else {
			id0 = addV(g, buf, v);
			
			sscanf(buf, "%d", &v);
			id1 = addV(g, buf, v);
			
			sscanf(buf, "%d", &v);
			id2 = addV(g, buf, v);
			
			addTriangle(g, id0, id1, id2);
			
			while(sscanf(buf, "%d", &v) == 1) {
				id1 = id2;
				id2 = addV(g, buf, v);
				addTriangle(g, id0, id1, id2);
			}
		}
	}
};

void Model :: readOBJ(std::string filename) {
//	mModel = glmReadOBJ(filename);
//	glmFacetNormals(mModel);
//	glmVertexNormals(mModel, 10);

	FILE*   file;
	std::string mtl = "default";
    
    /* open the file */
    file = fopen(filename.data(), "r");
    if (!file) {
        fprintf(stderr, "Model readOBJ failed: can't open data file \"%s\".\n",
            filename.data());
		return;
    }
	mPath = filename;
	
	Parser parser(file);
	
	/* create default group */
	Group * g = addGroup("default");    
	
	parser.readToken();
	while(parser.hasToken()) {
		switch(parser.buf[0]) {
			case 'u':
				g->material(mtl = parser.parseMaterial());
                break;
				
			case 'm':
				mMaterialLib = parser.parseMaterialLib();
				readMTL(mMaterialLib);
				break;
            
			case 'g':               /* group */
				/* eat up rest of line */
				g = addGroup(parser.parseGroup());
	            g->material(mtl);
				break;
				
			case 'v':               /* v, vn, vt */
				switch(parser.buf[1]) {
					case '\0':          /* vertex */
						parser.parseVertex(file);
						break;
					case 'n':           /* normal */
						parser.parseNormal(file);
						break;
					case 't':           /* texcoord */
						parser.parseTexcoord(file);
						break;
					default:
						printf("Model: Unknown token \"%s\".\n", parser.buf);
						return;
						break;
				}	
				break;	// end of case 'v'
			
			case 'f':               /* face */
				parser.parseFace(file, *g);
				break;
			
			default:
				/* eat up rest of line */
				parser.eatLine();
				break;
        }
	}
	
	printf("parsed %d vertices %d texcoords %d normals\n", parser.vertices.size()-1, parser.texcoords.size()-1, parser.normals.size()-1);
	
	printf("groups\n");
	std::map<std::string, Group>::iterator iter = mGroups.begin();
	while (iter != mGroups.end()) {	
		Group& gr = iter->second;
		
//		// create facet normals:
//		for (int i = 0; i < gr.mTriangles.size(); i++) {
//			Triangle& tri = gr.mTriangles[i];
//			
//			// the three points of the triangle:
//			GraphicsData::Vertex p0 = gr.data().vertices()[tri.indices[0]];
//			GraphicsData::Vertex p1 = gr.data().vertices()[tri.indices[1]];
//			GraphicsData::Vertex p2 = gr.data().vertices()[tri.indices[2]];
//			normal<float>(tri.normal, p0, p1, p2);
//		}
		
		//gr.data().unitize();
		gr.data().primitive(gfx::TRIANGLES);
		
		printf("%s name: %s (mt: %s)\n", iter->first.data(), gr.name().data(), gr.material().data());
		printf("\tindices: %d / vertices: %d\n", gr.data().indices().size(), gr.data().vertices().size());
		
		GraphicsData::Vertex min, max;
		gr.data().getBounds(min, max);
		printf("min %f %f %f max %f %f %f\n", min[0], min[1], min[2], max[0], max[1], max[2]);
		
		iter++;
	}
	
	/* close the file */
    fclose(file);
}
