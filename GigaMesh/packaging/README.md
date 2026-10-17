# Building and releasing the GigaMesh packages:

1. Update CITATION.cff

2. build_newsentry.sh: Should always be called first to set the version number within the changelog.

3. build_tarball.sh Should be executed on an older distribution e.g. Ubuntu 16.04, to provide binaries compatible with older systems.
   Vice versa you should not execute this script on a Manjaro system as the resulting binaries will run only with newest versions of the Qt libraries.

4. build_debian.sh: Same as for the tarball.

5. build_manjaro.sh: Executed on Manjaro. Best to be called last.

6. There is also an SVG for release announcements on social media.

Windows release is a portable version downloadable from https://ci.appveyor.com/project/rkuehl-iwr/gigamesh/ as artifact.
Same applies to the MacOS version, which is not signed (for now).
