////////////////////////////////////////////////////////////////////////////////
#include <polyfem/io/OBJData.hpp>
#include <polyfem/io/OBJReader.hpp>
#include <polyfem/io/OBJWriter.hpp>
#include <polyfem/io/USDReader.hpp>
#include <polyfem/io/USDWriter.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <filesystem>
#include <string>
////////////////////////////////////////////////////////////////////////////////

using namespace polyfem;
using namespace polyfem::io;
using Catch::Approx;

namespace
{
	std::string temp_usd(const std::string &name)
	{
		auto dir = std::filesystem::temp_directory_path() / "polyfem_usd_tests";
		std::filesystem::create_directories(dir);
		return (dir / name).string();
	}

	// A mixed-valence mesh: a quad (face 0) and a triangle (face 1) sharing an
	// edge, with per-corner UVs + normals and two material groups. Exercises
	// everything the round trip must preserve.
	OBJData make_sample()
	{
		OBJData d;
		d.V = {
			{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 1.0, 0.0},
			{0.0, 1.0, 0.0}, {2.0, 0.5, 0.0}};
		d.F = {{0, 1, 2, 3}, {1, 4, 2}}; // quad + triangle (no triangulation)

		d.VT = {{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}, {1.5, 0.5}};
		d.FT = {{0, 1, 2, 3}, {1, 4, 2}};

		d.VN = {{0.0, 0.0, 1.0}};
		d.FN = {{0, 0, 0, 0}, {0, 0, 0}};

		d.mtl_filename = "sample.mtl";

		OBJObject obj;
		obj.name = "SampleObject";
		OBJGroup g0;
		g0.name = "quad_grp";
		g0.material_name = "red";
		g0.face_indices = {0};
		OBJGroup g1;
		g1.name = "tri_grp";
		g1.material_name = "blue";
		g1.face_indices = {1};
		obj.groups = {g0, g1};
		d.objects = {obj};

		d.face_to_object = {0, 0};
		d.face_to_group = {0, 1}; // per-object local group index

		return d;
	}
} // namespace

TEST_CASE("usd mesh round trip preserves all data", "[usd][io]")
{
	const OBJData in = make_sample();
	const std::string path = temp_usd("roundtrip.usda");

	REQUIRE(USDWriter::write_with_groups(path, in));

	OBJData out;
	REQUIRE(USDReader::read_with_groups(path, out));

	// Vertices (positions + order).
	REQUIRE(out.V.size() == in.V.size());
	for (size_t i = 0; i < in.V.size(); ++i)
	{
		REQUIRE(out.V[i].size() >= 3);
		for (int c = 0; c < 3; ++c)
			REQUIRE(out.V[i][c] == Approx(in.V[i][c]));
	}

	// Faces: arbitrary valence preserved (quad stays a quad).
	REQUIRE(out.F == in.F);
	REQUIRE(out.F[0].size() == 4);
	REQUIRE(out.F[1].size() == 3);

	// UVs + tex-coord indices.
	REQUIRE(out.VT.size() == in.VT.size());
	for (size_t i = 0; i < in.VT.size(); ++i)
	{
		REQUIRE(out.VT[i][0] == Approx(in.VT[i][0]));
		REQUIRE(out.VT[i][1] == Approx(in.VT[i][1]));
	}
	REQUIRE(out.FT == in.FT);

	// Normals + normal indices.
	REQUIRE(out.VN.size() == in.VN.size());
	for (size_t i = 0; i < in.VN.size(); ++i)
		for (int c = 0; c < 3; ++c)
			REQUIRE(out.VN[i][c] == Approx(in.VN[i][c]));
	REQUIRE(out.FN == in.FN);

	// Groups / materials / object structure.
	REQUIRE(out.mtl_filename == in.mtl_filename);
	REQUIRE(out.objects.size() == 1);
	REQUIRE(out.objects[0].name == "SampleObject");
	REQUIRE(out.objects[0].groups.size() == 2);
	REQUIRE(out.objects[0].groups[0].name == "quad_grp");
	REQUIRE(out.objects[0].groups[0].material_name == "red");
	REQUIRE(out.objects[0].groups[0].face_indices == std::vector<int>{0});
	REQUIRE(out.objects[0].groups[1].name == "tri_grp");
	REQUIRE(out.objects[0].groups[1].material_name == "blue");
	REQUIRE(out.objects[0].groups[1].face_indices == std::vector<int>{1});

	// Recomputed face maps.
	REQUIRE(out.face_to_object == in.face_to_object);
	REQUIRE(out.face_to_group == in.face_to_group);
}

TEST_CASE("usd round trip matches OBJ on a real triangle mesh", "[usd][io]")
{
	// Load a bundled all-triangle garment via the OBJ path, round-trip it
	// through USD, and confirm the geometry survives unchanged.
	const std::string obj_in =
		std::string(POLYFEM_SOURCE_DIR) + "/tests/garment.obj";
	if (!std::filesystem::exists(obj_in))
		return; // fixture not present in this checkout

	OBJData via_obj;
	REQUIRE(OBJReader::read_with_groups(obj_in, via_obj));

	const std::string usd_path = temp_usd("garment.usda");
	REQUIRE(USDWriter::write_with_groups(usd_path, via_obj));

	OBJData via_usd;
	REQUIRE(USDReader::read_with_groups(usd_path, via_usd));

	REQUIRE(via_usd.V.size() == via_obj.V.size());
	REQUIRE(via_usd.F == via_obj.F); // valence + vertex indices identical
}
