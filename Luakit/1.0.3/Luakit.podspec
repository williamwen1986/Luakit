#
# Be sure to run `pod lib lint Luakit.podspec' to ensure this is a
# valid spec before submitting.
#
# Any lines starting with a # are optional, but their use is encouraged
# To learn more about a Podspec see https://guides.cocoapods.org/syntax/podspec.html
#

Pod::Spec.new do |s|
  s.name             = 'Luakit'
  s.version          = '1.0.3'
  s.summary          = 'A multi-platform solution in lua , you can develop your app, IOS or android app with this tool, truely write once , use everywhere.'

# This description is used to generate tags and improve search results.
#   * Think: What does it do? Why did you write it? What is the focus?
#   * Try to keep it short, snappy and to the point.
#   * Write the description between the DESC delimiters below.
#   * Finally, don't worry about the indent, CocoaPods strips it!

  s.description      = <<-DESC
A multi-platform solution in lua , you can develop your app, IOS or android app with this tool, truely write once , use everywhere.
                       DESC

  s.homepage         = 'https://github.com/williamwen1986/Luakit'
  s.license          = { :type => 'MIT', :file => 'LICENSE' }
  s.author           = { 'williamwen1986' => 'williamwen1986@gmail.com' }
  s.source           = { :git => 'https://github.com/williamwen1986/Luakit.git', :tag => s.version.to_s }

  s.ios.deployment_target = '6.0'
  s.source_files = 'LuaKitProject/IOSFrameWork/include/*.h'
  s.vendored_libraries  = 'LuaKitProject/IOSFrameWork/lib/*.a'
  s.libraries  = 'stdc++','z'

  
  # s.resource_bundles = {
  #   'Luakit' => ['Luakit/Assets/*.png']
  # }

  # s.public_header_files = 'Pod/Classes/**/*.h'
  # s.frameworks = 'UIKit', 'MapKit'
  # s.dependency 'AFNetworking', '~> 2.3'
end
