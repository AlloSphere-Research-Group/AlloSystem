require 'formula'

class Allosystem < Formula
  head 'https://github.com/AlloSphere-Research-Group/AlloSystem', :tag => 'cmake1'

  depends_on 'cmake' => :build
	depends_on 'assimp'
	depends_on 'freeimage'
	depends_on 'freetype'
	depends_on 'glew'
	depends_on 'lua'
	depends_on 'portaudio'
  depends_on 'libsndfile'

  def install
    system "cmake", ".", "-DNO_EXAMPLES=1", *std_cmake_args
    system "make"
    system "make install"
  end
end
