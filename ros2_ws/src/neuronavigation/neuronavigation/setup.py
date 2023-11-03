import os
from setuptools import find_packages, setup

# Search for InVesalius's internal packages in invesalius3 directory, allowing
# InVesalius to import them as, e.g., invesalius.gui, instead of invesalius3.invesalius.gui.
#
packages = find_packages(where='invesalius3')
package_dir = {s: 'invesalius3/' + s.replace('.', '/') for s in packages}

packages.append('invesalius3')
packages.append('bridge')

# XXX: setuptools doesn't directly support copying directories while retaining the
#   directory structure: the directory has to be specified for each copied file separately.
#   Therefore, manually generate the list of files to copy by traversing the directory
#   using os.walk().
#
#   Adapted from:
#
#   https://stackoverflow.com/questions/27829754/include-entire-directory-in-python-setup-py-data-files
#
def generate_data_files():
    dirs = (
        # Needed to run InVesalius in Docker or native Ubuntu.
        ('lib/python3.8/site-packages/locale', 'invesalius3/locale'),
        ('lib/python3.8/site-packages/icons', 'invesalius3/icons'),
        ('lib/python3.8/site-packages/invesalius_cy', 'invesalius3/invesalius_cy'),
        ('lib/python3.8/site-packages/samples', 'invesalius3/samples'),
        ('lib/python3.8/site-packages/navigation', 'invesalius3/navigation'),

        # HACK: It is not very clean to copy the same files into both python3.8 and -3.10 directories,
        #   but for now, it is the easiest way to support both Python versions.
        ('lib/python3.10/site-packages/locale', 'invesalius3/locale'),
        ('lib/python3.10/site-packages/icons', 'invesalius3/icons'),
        ('lib/python3.10/site-packages/invesalius_cy', 'invesalius3/invesalius_cy'),
        ('lib/python3.10/site-packages/samples', 'invesalius3/samples'),
        ('lib/python3.10/site-packages/navigation', 'invesalius3/navigation'),

        # HACK: Needed to run InVesalius in native Windows (outside Docker); for some reason, these files are
        # searched for in a different directory when running InVesalius in Windows, compared to Ubuntu.
        ('lib/site-packages/locale', 'invesalius3/locale'),
        ('lib/site-packages/icons', 'invesalius3/icons'),
        ('lib/site-packages/invesalius_cy', 'invesalius3/invesalius_cy'),
        ('lib/site-packages/samples', 'invesalius3/samples'),
        ('lib/site-packages/navigation', 'invesalius3/navigation'),
    )

    data_files = []
    for dest_dir, source_dir in dirs:
        for dirpath, _, filenames in os.walk(source_dir):
            dirpath_relative = os.path.relpath(dirpath, source_dir)
            install_dir = os.path.join(dest_dir, dirpath_relative)
            list_entry = (install_dir, [os.path.join(dirpath, f) for f in filenames if not f.startswith('.')])
            data_files.append(list_entry)

    data_files.append(('share/ament_index/resource_index/packages', ['resource/neuronavigation']))
    data_files.append(('share/neuronavigation', ['package.xml']))

    return data_files

setup(
    name='neuronavigation',
    version='0.0.1',
    packages=packages,
    package_dir=package_dir,
    install_requires=['setuptools'],
    data_files=generate_data_files(),
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
