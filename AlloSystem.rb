# must do 'brew install --HEAD ...'
#
require 'formula'

class Allosystem < Formula
  head 'https://github.com/mantaraya36/AlloSystem.git', :tag => 'homebrew_testing5'

  depends_on 'cmake' => :build
  depends_on 'libsndfile'
	depends_on 'portaudio'
	depends_on 'glew'
	depends_on 'assimp'
	depends_on 'freeimage'
	depends_on 'freetype'

  def install
    system "cmake", ".", "-DNO_EXAMPLES=1", *std_cmake_args
    system "make"
    system "make install"
  end
end
