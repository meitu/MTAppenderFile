Pod::Spec.new do |s|
  s.name         = "MTAppenderFile"
  s.version      = "0.4.2"
  s.summary      = "A simplified high-performance log component for *OS base on Tencent Mars xlog."

  s.description  = <<-DESC
    simplified high-performance log component base on mmap, with C & Objective-C API.
                   DESC

  s.homepage     = "https://github.com/meitu/MTAppenderFile"
  s.license      = {
    :type => 'Copyright',
    :text => <<-LICENSE
      © 2008 - present Meitu, Inc. All rights reserved.
    LICENSE
  }

  s.author       = { "Euan Chan" => "cqh@meitu.com" }

  s.platform     = :ios, "8.0"

  s.source       = { :git => "https://github.com/meitu/MTAppenderFile.git", :tag => "#{s.version}" }

  s.public_header_files = "loglib/MTAppenderFile.h", "loglib/mtaf_base.h" "loglib/mtaf_appender.h"
  s.source_files  = "loglib/**/*{h,hpp,m,mm,cpp,cc,c}", "comm/**/*.{h,hpp,m,mm,cpp,cc,c}"
  s.exclude_files = "MTAppenderFile/Exclude"

  s.requires_arc = false
  
  s.libraries = "z", "c++"

  s.pod_target_xcconfig = { 'OTHER_LDFLAGS' => '-lc++' }

end
