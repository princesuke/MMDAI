require File.dirname(__FILE__) + '/cmake.rb'
require File.dirname(__FILE__) + '/svn.rb'

module Mmdai

class Bullet < Thor
  include Build::CMake
  include VCS::SVN

  desc "debug", "build bullet for debug"
  method_options :flag => :boolean
  def debug
    checkout
    invoke_build :debug
  end
  desc "release", "build bullet for release"
  method_options :flag => :boolean
  def release
    checkout
    invoke_build :release
  end

protected
  def get_uri
    "http://bullet.googlecode.com/svn/tags/bullet-2.77"
  end

  def get_directory_name
    "bullet-src"
  end

  def get_build_options(build_type, extra_options)
    return {
      :build_demos => false,
      :build_extras => false,
      :install_libs => true,
      :use_glut => false
    }
  end

end

end
