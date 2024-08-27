import os
from setuptools import find_packages, setup

# Search for InVesalius's internal packages in invesalius3 directory, allowing
# InVesalius to import them as, e.g., invesalius.gui, instead of invesalius3.invesalius.gui.
#
packages = find_packages(where='tms_robot_control')
package_dir = {s: 'tms_robot_control/' + s.replace('.', '/') for s in packages}

packages.append('tms_robot_control')
packages.append('bridge_robot')

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
        ('lib/python3.8/site-packages/tms_robot_control', 'tms_robot_control'),
        ('lib/python3.8/site-packages/robot', 'tms_robot_control/robot'),

        # HACK: It is not very clean to copy the same files into both python3.8 and -3.10 directories,
        #   but for now, it is the easiest way to support both Python versions.
        ('lib/python3.10/site-packages/tms_robot_control', 'tms_robot_control'),
        ('lib/python3.10/site-packages/robot', 'tms_robot_control/robot'),

        # HACK: Needed to run InVesalius in native Windows (outside Docker); for some reason, these files are
        # searched for in a different directory when running InVesalius in Windows, compared to Ubuntu.
        ('lib/site-packages/tms_robot_control', 'tms_robot_control'),
        ('lib/site-packages/robot', 'tms_robot_control/robot'),
    )

    data_files = []
    for dest_dir, source_dir in dirs:
        for dirpath, _, filenames in os.walk(source_dir):
            dirpath_relative = os.path.relpath(dirpath, source_dir)
            install_dir = os.path.join(dest_dir, dirpath_relative)
            list_entry = (install_dir, [os.path.join(dirpath, f) for f in filenames])
            data_files.append(list_entry)

    data_files.append(('share/ament_index/resource_index/packages', ['resource/robot']))
    data_files.append(('share/robot', ['package.xml']))

    return data_files

setup(
    name='robot',
    version='0.0.1',
    packages=packages,
    package_dir=package_dir,
    install_requires=['setuptools'],
    data_files=generate_data_files(),
    zip_safe=True,
    author='Renan Matsuda',
    author_email='renan.matsuda@aalto.fi',
    description='Python packages for tms_robot_control',
    license='TODO',
    entry_points={
        'console_scripts': [
            "start = bridge_robot.bridge_robot:main",
        ],
    },
)
