CPPFLAGS=-Wall -O3 -I/usr/include/libxml2

SOURCE= colladatool.cpp ColladaNode.cpp ColladaSource.cpp ColladaTriangles.cpp ColladaVertices.cpp ColladaAnimation.cpp ColladaMaterial.cpp ColladaBindMaterial.cpp ColladaController.cpp ColladaEffect.cpp ColladaFloatArray.cpp ColladaGeometry.cpp ColladaImage.cpp ColladaIDREFArray.cpp


OBJ = $(SOURCE:%.cpp=%.o)


all:colladatool

objc: $(SOURCE)
	$(CXX) $(CPPFLAGS) $(SOURCE) -c 


colladatool: $(OBJ) 
	$(CXX) -lxml2 $(CPPFLAGS) $(OBJ) -o colladatool


clean:
	$(RM) *.o *~ colladatool
