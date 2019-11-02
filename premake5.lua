-- MockbaModular by Mockba the Borg
-- Premake Script
-- Requires premake5.exe to be somewhere reachable via PATH
-- https://premake.github.io/
--
workspace "MockbaVS"
	location "VStudio"
	configurations { "Release" }
	platforms { "X64" }
	startproject "MockbaVS"
	pic "On"
	systemversion "10.0.17763.0"
	characterset "ASCII"

project "MockbaVS"
	kind "SharedLib"
	targetname "plugin"
	language "C++"
	targetdir "."
	files { "src/**" }
	vpaths {
		["Headers/*"] = { "**.hpp", "**.h" }
	}
	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
	includedirs { "../Rack-SDK/include", "../Rack-SDK/dep/include" }
	
-- Cleanup
if _ACTION == "clean" then
	os.rmdir("VStudio");
	os.rmdir("build");
	os.rmdir("dep");
	os.rmdir("dist");
	os.remove("plugin.dll");
end
