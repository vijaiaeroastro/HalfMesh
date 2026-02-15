#include "halfMesh.hpp"

#include <chrono>
#include <cstdlib>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using Clock = std::chrono::steady_clock;

halfMesh::triMesh build_grid_mesh(const int nx, const int ny) {
    halfMesh::triMesh mesh;
    std::vector<halfMesh::vertexPtr> verts(static_cast<size_t>(nx + 1) * static_cast<size_t>(ny + 1));
    auto idx = [nx](const int x, const int y) {
        return static_cast<size_t>(y) * static_cast<size_t>(nx + 1) + static_cast<size_t>(x);
    };

    for (int y = 0; y <= ny; ++y) {
        for (int x = 0; x <= nx; ++x) {
            verts[idx(x, y)] = mesh.add_vertex(static_cast<double>(x), static_cast<double>(y), 0.0);
        }
    }

    for (int y = 0; y < ny; ++y) {
        for (int x = 0; x < nx; ++x) {
            const auto v00 = verts[idx(x, y)];
            const auto v10 = verts[idx(x + 1, y)];
            const auto v01 = verts[idx(x, y + 1)];
            const auto v11 = verts[idx(x + 1, y + 1)];

            mesh.add_face(v00, v10, v11);
            mesh.add_face(v00, v11, v01);
        }
    }
    mesh.complete_mesh();
    return mesh;
}

halfMesh::triMesh build_quad_soup_mesh(const size_t quad_count) {
    halfMesh::triMesh mesh;
    for (size_t i = 0; i < quad_count; ++i) {
        const double x0 = static_cast<double>(i) * 3.0;
        const auto a = mesh.add_vertex(x0 + 0.0, 0.0, 0.0);
        const auto b = mesh.add_vertex(x0 + 1.0, 0.0, 0.0);
        const auto c = mesh.add_vertex(x0 + 0.0, 1.0, 0.0);
        const auto d = mesh.add_vertex(x0 + 1.0, 1.0, 0.0);
        mesh.add_face(a, b, c);
        mesh.add_face(b, a, d);
    }
    mesh.complete_mesh();
    return mesh;
}

struct BenchStats {
    std::string name;
    size_t attempted = 0;
    size_t succeeded = 0;
    double seconds = 0.0;
};

struct BenchOptions {
    int nx = 32;
    int ny = 32;
    size_t split_iters = 300;
    size_t collapse_iters = 200;
    size_t flip_iters = 1000;
    bool json = false;
};

void apply_profile(BenchOptions &opts, const std::string &profile) {
    if (profile == "quick") {
        opts.nx = 32;
        opts.ny = 32;
        opts.split_iters = 300;
        opts.collapse_iters = 200;
        opts.flip_iters = 1000;
        return;
    }
    if (profile == "stress") {
        opts.nx = 64;
        opts.ny = 64;
        opts.split_iters = 1200;
        opts.collapse_iters = 800;
        opts.flip_iters = 4000;
        return;
    }
    throw std::runtime_error("Unknown profile: " + profile + " (expected quick|stress)");
}

[[noreturn]] void print_usage_and_exit(const char *argv0) {
    std::cerr
        << "Usage: " << argv0 << " [options]\n"
        << "Options:\n"
        << "  --profile <quick|stress>\n"
        << "  --nx <int>\n"
        << "  --ny <int>\n"
        << "  --split-iters <int>\n"
        << "  --collapse-iters <int>\n"
        << "  --flip-iters <int>\n"
        << "  --json\n"
        << "  --help\n";
    std::exit(2);
}

BenchOptions parse_options(const int argc, char **argv) {
    BenchOptions opts{};
    apply_profile(opts, "quick");

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        auto require_value = [&](const char *name) -> std::string {
            if (i + 1 >= argc) {
                throw std::runtime_error(std::string("Missing value for ") + name);
            }
            return argv[++i];
        };
        if (arg == "--help") {
            print_usage_and_exit(argv[0]);
        } else if (arg == "--json") {
            opts.json = true;
        } else if (arg == "--profile") {
            apply_profile(opts, require_value("--profile"));
        } else if (arg == "--nx") {
            opts.nx = std::stoi(require_value("--nx"));
        } else if (arg == "--ny") {
            opts.ny = std::stoi(require_value("--ny"));
        } else if (arg == "--split-iters") {
            opts.split_iters = static_cast<size_t>(std::stoull(require_value("--split-iters")));
        } else if (arg == "--collapse-iters") {
            opts.collapse_iters = static_cast<size_t>(std::stoull(require_value("--collapse-iters")));
        } else if (arg == "--flip-iters") {
            opts.flip_iters = static_cast<size_t>(std::stoull(require_value("--flip-iters")));
        } else {
            throw std::runtime_error("Unknown argument: " + arg);
        }
    }

    if (opts.nx <= 0 || opts.ny <= 0) {
        throw std::runtime_error("nx and ny must be > 0");
    }
    return opts;
}

BenchStats bench_split(const int nx, const int ny, const size_t iterations) {
    auto mesh = build_grid_mesh(nx, ny);
    BenchStats stats{"split_edge", iterations, 0, 0.0};

    const auto t0 = Clock::now();
    for (size_t i = 0; i < iterations; ++i) {
        halfMesh::edgePtr candidate = nullptr;
        for (const auto &e: mesh.get_edges()) {
            if (mesh.can_split(e)) {
                candidate = e;
                break;
            }
        }
        if (!candidate) break;
        if (mesh.split_edge(candidate).ok) {
            ++stats.succeeded;
        } else {
            break;
        }
    }
    mesh.complete_mesh();
    const auto t1 = Clock::now();
    stats.seconds = std::chrono::duration<double>(t1 - t0).count();
    return stats;
}

BenchStats bench_collapse(const int nx, const int ny, const size_t iterations) {
    auto mesh = build_grid_mesh(nx, ny);
    BenchStats stats{"collapse_edge", iterations, 0, 0.0};

    const auto t0 = Clock::now();
    for (size_t i = 0; i < iterations; ++i) {
        halfMesh::edgePtr candidate = nullptr;
        halfMesh::vertexPtr target = nullptr;
        for (const auto &e: mesh.get_edges()) {
            const auto v0 = e->get_vertex_one();
            const auto v1 = e->get_vertex_two();
            if (v0 && mesh.can_collapse(e, v0)) {
                candidate = e;
                target = v0;
                break;
            }
            if (v1 && mesh.can_collapse(e, v1)) {
                candidate = e;
                target = v1;
                break;
            }
        }
        if (!candidate || !target) break;
        if (mesh.collapse_edge(candidate, target).ok) {
            ++stats.succeeded;
        } else {
            break;
        }
    }
    mesh.complete_mesh();
    const auto t1 = Clock::now();
    stats.seconds = std::chrono::duration<double>(t1 - t0).count();
    return stats;
}

BenchStats bench_flip(const int nx, const int ny, const size_t iterations) {
    const size_t quads = std::max<size_t>(iterations, static_cast<size_t>(nx * ny));
    auto mesh = build_quad_soup_mesh(quads);
    BenchStats stats{"flip_edge", iterations, 0, 0.0};

    const auto t0 = Clock::now();
    for (size_t i = 0; i < iterations; ++i) {
        halfMesh::edgePtr candidate = nullptr;
        for (const auto &e: mesh.get_edges()) {
            if (mesh.can_flip(e)) {
                candidate = e;
                break;
            }
        }
        if (!candidate) break;
        if (mesh.flip_edge(candidate).ok) {
            ++stats.succeeded;
        } else {
            break;
        }
    }
    mesh.complete_mesh();
    const auto t1 = Clock::now();
    stats.seconds = std::chrono::duration<double>(t1 - t0).count();
    return stats;
}

void print_stats(const BenchStats &stats) {
    const double ops_per_sec = (stats.seconds > 0.0)
                                   ? static_cast<double>(stats.succeeded) / stats.seconds
                                   : 0.0;
    std::cout << std::left << std::setw(14) << stats.name
              << " attempted=" << std::setw(6) << stats.attempted
              << " succeeded=" << std::setw(6) << stats.succeeded
              << " time=" << std::fixed << std::setprecision(6) << stats.seconds << "s"
              << " ops/s=" << std::fixed << std::setprecision(2) << ops_per_sec << '\n';
}

double ops_per_second(const BenchStats &stats) {
    return (stats.seconds > 0.0) ? static_cast<double>(stats.succeeded) / stats.seconds : 0.0;
}

void print_json(const BenchOptions &opts,
                const BenchStats &split,
                const BenchStats &collapse,
                const BenchStats &flip) {
    std::cout
        << "{\n"
        << "  \"grid\": {\"nx\": " << opts.nx << ", \"ny\": " << opts.ny << "},\n"
        << "  \"results\": [\n"
        << "    {\"name\": \"" << split.name << "\", \"attempted\": " << split.attempted
        << ", \"succeeded\": " << split.succeeded
        << ", \"seconds\": " << std::fixed << std::setprecision(6) << split.seconds
        << ", \"ops_per_sec\": " << std::fixed << std::setprecision(2) << ops_per_second(split) << "},\n"
        << "    {\"name\": \"" << collapse.name << "\", \"attempted\": " << collapse.attempted
        << ", \"succeeded\": " << collapse.succeeded
        << ", \"seconds\": " << std::fixed << std::setprecision(6) << collapse.seconds
        << ", \"ops_per_sec\": " << std::fixed << std::setprecision(2) << ops_per_second(collapse) << "},\n"
        << "    {\"name\": \"" << flip.name << "\", \"attempted\": " << flip.attempted
        << ", \"succeeded\": " << flip.succeeded
        << ", \"seconds\": " << std::fixed << std::setprecision(6) << flip.seconds
        << ", \"ops_per_sec\": " << std::fixed << std::setprecision(2) << ops_per_second(flip) << "}\n"
        << "  ]\n"
        << "}\n";
}
} // namespace

int main(int argc, char **argv) {
    BenchOptions opts{};
    try {
        opts = parse_options(argc, argv);
    } catch (const std::exception &ex) {
        std::cerr << ex.what() << '\n';
        print_usage_and_exit(argv[0]);
    }

    const auto split = bench_split(opts.nx, opts.ny, opts.split_iters);
    const auto collapse = bench_collapse(opts.nx, opts.ny, opts.collapse_iters);
    const auto flip = bench_flip(opts.nx, opts.ny, opts.flip_iters);

    if (opts.json) {
        print_json(opts, split, collapse, flip);
        return 0;
    }

    std::cout << "halfMesh local edit benchmark\n";
    std::cout << "grid=" << opts.nx << "x" << opts.ny << '\n';
    print_stats(split);
    print_stats(collapse);
    print_stats(flip);
    return 0;
}
