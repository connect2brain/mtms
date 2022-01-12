from setuptools import find_packages, setup

# Search for InVesalius's internal packages in invesalius3 directory, allowing
# InVesalius to import them as, e.g., invesalius.gui, instead of invesalius3.invesalius.gui.
#
packages = find_packages(where='invesalius3')
package_dir = {s: 'invesalius3/' + s.replace('.', '/') for s in packages}

packages.append('invesalius3')
packages.append('bridge')

setup(
    # Use pbr extension to setuptools. Without that, copying data files, e.g.,
    # the directory structure in invesalius_pkg/locale, is hard because
    # 'data_files' parameter in setuptools does not support copying directories;
    # it only supports copying single files without retaining the directory
    # structure. See, e.g.,
    #
    # https://stackoverflow.com/questions/27829754/include-entire-directory-in-\
    # python-setup-py-data-files
    #
    setup_requires=['pbr'],
    pbr=True,

    name='neuronavigation_pkg',
    version='0.0.1',
    packages=packages,
    package_dir=package_dir,
    install_requires=['setuptools'],

    # XXX: Does not seems to work: generates a file 'not-zip-safe' to the egg-info
    #   directory. Most likely due to pbr conflicting with setuptools.
    #
    zip_safe=True,

    author='Olli-Pekka Kahilakoski',
    author_email='olli-pekka.kahilakoski@aalto.fi',
    description='Python packages for neuronavigation',
    license='TODO',
    entry_points={
        'console_scripts': [
            "start = bridge.bridge:main",
        ],
    },
)
