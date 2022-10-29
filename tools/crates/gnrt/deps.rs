// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! Utilities to process `cargo metadata` dependency graph.

use crate::crates::{self, Epoch};
use crate::platforms::{self, Platform, PlatformSet};

use std::collections::{hash_map::Entry, HashMap, HashSet};
use std::iter;
use std::path::PathBuf;

pub use cargo_metadata::DependencyKind;
pub use semver::Version;

/// A single transitive dependency of a root crate. Includes information needed
/// for generating build files later.
#[derive(Clone, Debug)]
pub struct Package {
    /// The package name as used by cargo.
    pub package_name: String,
    /// The package version as used by cargo.
    pub version: Version,
    /// This package's dependencies. Each element cross-references another
    /// `Package` by name and version.
    pub dependencies: Vec<DepOfDep>,
    /// Same as the above, but for build script deps.
    pub build_dependencies: Vec<DepOfDep>,
    /// Same as the above, but for test deps.
    pub dev_dependencies: Vec<DepOfDep>,
    /// A package can be depended upon in different ways: as a normal
    /// dependency, just for build scripts, or just for tests. `kinds` contains
    /// an entry for each way this package is depended on.
    pub dependency_kinds: HashMap<DependencyKind, PerKindInfo>,
    /// The package's lib target, or `None` if it doesn't have one.
    pub lib_target: Option<LibTarget>,
    /// List of binaries provided by the package.
    pub bin_targets: Vec<BinTarget>,
    /// The build script's absolute path, or `None` if the package does not use
    /// one.
    pub build_script: Option<PathBuf>,
    /// The path in the dependency graph to this package. This is intended for
    /// human consumption when debugging missing packages.
    pub dependency_path: Vec<String>,
    /// Whether the source is a local path. Is `false` if cargo resolved this
    /// dependency from a registry (e.g. crates.io) or git. If `false` the
    /// package may still be locally vendored through cargo configuration (see
    /// https://doc.rust-lang.org/cargo/reference/source-replacement.html)
    pub is_local: bool,
    /// Whether this package is a member of the cargo workspace the metadata
    /// came from, as opposed to a third-party dependency.
    pub is_workspace_member: bool,
}

impl Package {
    pub fn third_party_crate_id(&self) -> crates::ChromiumVendoredCrate {
        crates::ChromiumVendoredCrate {
            name: self.package_name.clone(),
            epoch: Epoch::from_version(&self.version),
        }
    }
}

/// A dependency of a `Package`. Cross-references another `Package` entry in the
/// resolved list.
#[derive(Clone, Debug, Eq, PartialEq)]
pub struct DepOfDep {
    /// This dependency's package name as used by cargo.
    pub package_name: String,
    /// The resolved version of this dependency.
    pub version: Version,
    /// A platform constraint for this dependency, or `None` if it's used on all
    /// platforms.
    pub platform: Option<Platform>,
}

impl DepOfDep {
    pub fn third_party_crate_id(&self) -> crates::ChromiumVendoredCrate {
        crates::ChromiumVendoredCrate {
            name: self.package_name.clone(),
            epoch: Epoch::from_version(&self.version),
        }
    }
}

/// Information specific to the dependency kind: for normal, build script, or
/// test dependencies.
#[derive(Clone, Debug)]
pub struct PerKindInfo {
    /// The set of platforms this kind is needed on.
    pub platforms: PlatformSet,
    /// The resovled feature set for this kind.
    pub features: Vec<String>,
}

/// Description of a package's lib target.
#[derive(Clone, Debug)]
pub struct LibTarget {
    /// The absolute path of the lib target's `lib.rs`.
    pub root: PathBuf,
    /// The type of the lib target. This is "rlib" for normal dependencies and
    /// "proc-macro" for proc macros.
    pub lib_type: LibType,
}

/// A binary provided by a package.
#[derive(Clone, Debug)]
pub struct BinTarget {
    /// The absolute path of the binary's root source file (e.g. `main.rs`).
    pub root: PathBuf,
    /// The binary name.
    pub name: String,
}

/// The type of lib target. Only includes types supported by this tool.
#[derive(Clone, Copy, Debug)]
pub enum LibType {
    /// A normal Rust rlib library.
    Rlib,
    /// A Rust dynamic library. See
    /// https://doc.rust-lang.org/reference/linkage.html for details and the
    /// distinction between dylib and cdylib.
    Dylib,
    /// A C-compatible dynamic library. See
    /// https://doc.rust-lang.org/reference/linkage.html for details and the
    /// distinction between dylib and cdylib.
    Cdylib,
    /// A procedural macro.
    ProcMacro,
}

impl std::fmt::Display for LibType {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match *self {
            Self::Rlib => f.write_str("rlib"),
            Self::Dylib => f.write_str("dylib"),
            Self::Cdylib => f.write_str("cdylib"),
            Self::ProcMacro => f.write_str("proc-macro"),
        }
    }
}

/// Process the dependency graph in `metadata` to a flat list of transitive
/// dependencies. Each element in the result corresponds to a cargo package. A
/// package may have multiple crates, each of which corresponds to a single
/// rustc invocation: e.g. a package may have a lib crate as well as multiple
/// binary crates.
///
/// Optionally, `roots` specifies from which packages to traverse the dependency
/// graph (likely the root packages to generate build files for). This overrides
/// the usual behavior, which traverses from all workspace members and the root
/// workspace package. The package names in `roots` should still only contain
/// workspace members.
pub fn collect_dependencies(
    metadata: &cargo_metadata::Metadata,
    roots: Option<Vec<String>>,
) -> Vec<Package> {
    // The metadata is split into two parts:
    // 1. A list of packages and associated info: targets (e.g. lib, bin,
    //    tests), source path, etc. This includes all workspace members and all
    //    transitive dependencies. Deps are not filtered based on platform or
    //    features: it is the maximal set of dependencies.
    // 2. Resolved dependency graph. There is a node for each package pointing
    //    to its dependencies in each configuration (normal, build, dev), and
    //    the resolved feature set. This includes platform-specific info so one
    //    can filter based on target platform. Nodes include an ID that uniquely
    //    refers to a package in both (1) and (2).
    //
    // We need info from both parts. Traversing the graph tells us exactly which
    // crates are needed for a given configuration and platform. In the process,
    // we must collect package IDs then look up other data in (1).
    //
    // Note the difference between "packages" and "crates" as described in
    // https://doc.rust-lang.org/book/ch07-01-packages-and-crates.html

    // `metadata`'s structures are flattened into lists. Make it easy to index
    // by package ID.
    let dep_graph: MetadataGraph = build_graph(metadata);

    // `cargo metadata`'s resolved dependency graph.
    let resolved_graph: &cargo_metadata::Resolve = metadata.resolve.as_ref().unwrap();

    // The ID of the fake root package. Do not include it in the dependency list
    // since it is not actually built.
    let fake_root: &cargo_metadata::PackageId = resolved_graph.root.as_ref().unwrap();

    // `explore_node`, our recursive depth-first traversal function, needs to
    // share state between stack frames. Construct the shared state.
    let mut traversal_state = TraversalState {
        dep_graph: &dep_graph,
        root: fake_root,
        visited: HashSet::new(),
        path: Vec::new(),
        dependencies: HashMap::new(),
    };

    let traversal_roots: Vec<&cargo_metadata::PackageId> = match roots {
        Some(roots) => metadata
            .packages
            .iter()
            .filter_map(|pkg| if roots.contains(&pkg.name) { Some(&pkg.id) } else { None })
            .collect(),
        None => dep_graph.roots.clone(),
    };

    // Do a depth-first traversal of the graph to find all relevant
    // dependencies. Start from each workspace package ("chromium" and
    // additional binary members used in the build).
    for root_id in traversal_roots.iter() {
        let node_map: &HashMap<&cargo_metadata::PackageId, &cargo_metadata::Node> =
            &dep_graph.nodes;
        explore_node(&mut traversal_state, node_map.get(*root_id).unwrap());
    }

    // `traversal_state.dependencies` is the output of `explore_node`. Pull it
    // out for processing.
    let mut dependencies = traversal_state.dependencies;

    // Fill in the per-package data for each dependency.
    for (id, dep) in dependencies.iter_mut() {
        let node: &cargo_metadata::Node = traversal_state.dep_graph.nodes.get(id).unwrap();
        let package: &cargo_metadata::Package = traversal_state.dep_graph.packages.get(id).unwrap();

        dep.package_name = package.name.clone();

        // TODO(crbug.com/1291994): Resolve features independently per kind
        // and platform. This may require using the unstable unit-graph feature:
        // https://doc.rust-lang.org/cargo/reference/unstable.html#unit-graph
        for (_, mut kind_info) in dep.dependency_kinds.iter_mut() {
            kind_info.features = node.features.clone();
            // Remove "default" feature to match behavior of crates.py. Note
            // that this is technically not correct since a crate's code may
            // choose to check "default" directly, but virtually none actually
            // do this.
            //
            // TODO(crbug.com/1291994): Revisit this behavior and maybe keep
            // "default" features.
            if let Some(pos) = kind_info.features.iter().position(|x| x == "default") {
                kind_info.features.remove(pos);
            }
        }

        for target in package.targets.iter() {
            let src_root = target.src_path.clone().into_std_path_buf();
            let target_type = match target.kind.iter().find_map(|s| TargetType::from_name(s)) {
                Some(target_type) => target_type,
                // Skip other targets, such as test, example, etc.
                None => continue,
            };

            match target_type {
                TargetType::Lib(lib_type) => {
                    // There can only be one lib target.
                    assert!(
                        dep.lib_target.is_none(),
                        "found duplicate lib target:\n{:?}\n{:?}",
                        dep.lib_target,
                        target
                    );
                    dep.lib_target = Some(LibTarget { root: src_root, lib_type });
                }
                TargetType::Bin => {
                    dep.bin_targets.push(BinTarget { root: src_root, name: target.name.clone() });
                }
                TargetType::BuildScript => {
                    assert_eq!(
                        dep.build_script, None,
                        "found duplicate build script target {:?}",
                        target
                    );
                    dep.build_script = Some(src_root);
                }
            }
        }

        dep.version = package.version.clone();

        // Collect this package's list of resolved dependencies which will be
        // needed for build file generation later.
        for node_dep in iter_node_deps(node) {
            let dep_pkg = dep_graph.packages.get(node_dep.pkg).unwrap();
            let mut platform = node_dep.target;
            if let Some(p) = platform {
                assert!(platforms::matches_supported_target(&p));
                platform = platforms::filter_unsupported_platform_terms(p);
            }
            let dep_of_dep = DepOfDep {
                package_name: dep_pkg.name.clone(),
                version: dep_pkg.version.clone(),
                platform,
            };

            match node_dep.kind {
                DependencyKind::Normal => dep.dependencies.push(dep_of_dep),
                DependencyKind::Build => dep.build_dependencies.push(dep_of_dep),
                DependencyKind::Development => dep.dev_dependencies.push(dep_of_dep),
                DependencyKind::Unknown => unreachable!(),
            }
        }

        // Make sure the package comes from our vendored source. If not, report
        // the error for later.
        dep.is_local = package.source == None;

        // Determine whether it's a workspace member or third-party dependency.
        dep.is_workspace_member = dep_graph.workspace_members.contains(&package.id);
    }

    // Return a flat list of dependencies.
    dependencies.into_iter().map(|(_, v)| v).collect()
}

/// Graph traversal state shared by recursive calls of `explore_node`.
struct TraversalState<'a> {
    /// The graph from "cargo metadata", processed for indexing by package id.
    dep_graph: &'a MetadataGraph<'a>,
    /// The fake root package that we exclude from `dependencies`.
    root: &'a cargo_metadata::PackageId,
    /// Set of packages already visited by `explore_node`.
    visited: HashSet<&'a cargo_metadata::PackageId>,
    /// The path of package IDs to the current node. For human consumption.
    path: Vec<String>,
    /// The final set of dependencies.
    dependencies: HashMap<&'a cargo_metadata::PackageId, Package>,
}

/// Recursively explore a particular node in the dependency graph. Fills data in
/// `state`. The final output is in `state.dependencies`.
fn explore_node<'a>(state: &mut TraversalState<'a>, node: &'a cargo_metadata::Node) {
    // Mark the node as visited, or continue if it's already visited.
    if !state.visited.insert(&node.id) {
        return;
    }

    // Helper to insert a placeholder `Dependency` into a map. We fill in the
    // fields later.
    let init_dep = |path| Package {
        package_name: String::new(),
        version: Version::new(0, 0, 0),
        dependencies: Vec::new(),
        build_dependencies: Vec::new(),
        dev_dependencies: Vec::new(),
        dependency_kinds: HashMap::new(),
        lib_target: None,
        bin_targets: Vec::new(),
        build_script: None,
        dependency_path: path,
        is_local: false,
        is_workspace_member: false,
    };

    state.path.push(node.id.repr.clone());

    // Each node contains a list of enabled features plus a list of
    // dependencies. Each dependency has a platform filter if applicable.
    for dep_edge in iter_node_deps(node) {
        // Explore the target of this edge next. Note that we may visit the same
        // node multiple times, but this is OK since we'll skip it in the
        // recursive call.
        let target_node: &cargo_metadata::Node = state.dep_graph.nodes.get(&dep_edge.pkg).unwrap();
        explore_node(state, target_node);

        // Merge this with the existing entry for the dep.
        let dep: &mut Package =
            state.dependencies.entry(dep_edge.pkg).or_insert_with(|| init_dep(state.path.clone()));
        let info: &mut PerKindInfo = dep
            .dependency_kinds
            .entry(dep_edge.kind)
            .or_insert(PerKindInfo { platforms: PlatformSet::empty(), features: Vec::new() });
        info.platforms.add(dep_edge.target);
    }

    state.path.pop();

    // Initialize the dependency entry for this node's package if it's not our
    // fake root.
    if &node.id != state.root {
        state.dependencies.entry(&node.id).or_insert_with(|| init_dep(state.path.clone()));
    }
}

struct DependencyEdge<'a> {
    pkg: &'a cargo_metadata::PackageId,
    kind: DependencyKind,
    target: Option<Platform>,
}

/// Iterates over the dependencies of `node`, filtering out platforms we don't
/// support.
fn iter_node_deps(node: &cargo_metadata::Node) -> impl Iterator<Item = DependencyEdge<'_>> + '_ {
    node.deps
        .iter()
        .map(|node_dep| {
            // Each NodeDep has information about the package depended on, as well
            // as the kinds of dependence: as a normal, build script, or test
            // dependency. For each kind there is an optional platform filter.
            //
            // Filter out kinds for unsupported platforms while mapping the
            // dependency edges to our own type.
            node_dep.dep_kinds.iter().filter_map(|dep_kind_info| {
                // Filter if it's for a platform we don't support.
                match &dep_kind_info.target {
                    None => (),
                    Some(platform) => {
                        if !platforms::matches_supported_target(platform) {
                            return None;
                        }
                    }
                };

                Some(DependencyEdge {
                    pkg: &node_dep.pkg,
                    kind: dep_kind_info.kind,
                    target: dep_kind_info.target.clone(),
                })
            })
        })
        .flatten()
}

/// Indexable representation of the `cargo_metadata::Metadata` fields we need.
struct MetadataGraph<'a> {
    nodes: HashMap<&'a cargo_metadata::PackageId, &'a cargo_metadata::Node>,
    packages: HashMap<&'a cargo_metadata::PackageId, &'a cargo_metadata::Package>,
    workspace_members: HashSet<&'a cargo_metadata::PackageId>,
    roots: Vec<&'a cargo_metadata::PackageId>,
}

/// Convert the flat lists in `metadata` to maps indexable by PackageId.
fn build_graph<'a>(metadata: &'a cargo_metadata::Metadata) -> MetadataGraph<'a> {
    // `metadata` always has `resolve` unless cargo was explicitly asked not to
    // output the dependency graph.
    let resolve = metadata.resolve.as_ref().unwrap();
    let mut graph = HashMap::new();
    for node in resolve.nodes.iter() {
        match graph.entry(&node.id) {
            Entry::Vacant(e) => e.insert(node),
            Entry::Occupied(_) => panic!("duplicate entries in dependency graph"),
        };
    }

    let packages = metadata.packages.iter().map(|p| (&p.id, p)).collect();

    let roots = iter::once(resolve.root.as_ref().unwrap())
        .chain(metadata.workspace_members.iter())
        .collect();

    MetadataGraph {
        nodes: graph,
        packages,
        workspace_members: metadata.workspace_members.iter().collect(),
        roots,
    }
}

/// A crate target type we support.
#[derive(Clone, Copy, Debug)]
enum TargetType {
    Lib(LibType),
    Bin,
    BuildScript,
}

impl TargetType {
    fn from_name(name: &str) -> Option<Self> {
        match name {
            "lib" | "rlib" => Some(Self::Lib(LibType::Rlib)),
            "dylib" => Some(Self::Lib(LibType::Dylib)),
            "cdylib" => Some(Self::Lib(LibType::Cdylib)),
            "bin" => Some(Self::Bin),
            "custom-build" => Some(Self::BuildScript),
            "proc-macro" => Some(Self::Lib(LibType::ProcMacro)),
            _ => None,
        }
    }
}

impl std::fmt::Display for TargetType {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match *self {
            Self::Lib(typ) => typ.fmt(f),
            Self::Bin => f.write_str("bin"),
            Self::BuildScript => f.write_str("custom-build"),
        }
    }
}
