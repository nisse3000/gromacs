Bugs fixed
^^^^^^^^^^

.. Note to developers!
   Please use """"""" to underline the individual entries for fixed issues in the subfolders,
   otherwise the formatting on the webpage is messed up.
   Also, please use the syntax :issue:`number` to reference issues on GitLab, without the
   a space between the colon and number!

Fixed exported libgromacs CMake target
""""""""""""""""""""""""""""""""""""""

Update the exported libgromacs CMake target to not depend on non-
existing include paths and add GMX_DOUBLE define to interface
definitions. The target now gets exported into the Gromacs namespace.

:issue:`3468`

Fixed unsolicited changing of atom names in pdb file
""""""""""""""""""""""""""""""""""""""""""""""""""""

Remove functions to change atoms names when reading 
and writing pdb files. This affected naming of
H atoms in particular.

:issue:`3469`

Correct excluded perturbed interactions beyond the non-bonded cut-off distance
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""

With free-energy calculations without coupling of intermolecular interactions,
non-bonded pair interactions at distance longer than the cut-off distance can
be excluded. These interactions would still have PME long-range contributions.
The contributions are now removed. In addition, mdrun will stop with a fatal
error when interactions beyond the pair-list cut-off are present.

:issue:`3403`
:issue:`3808`

Corrected AWH initial histogram size
""""""""""""""""""""""""""""""""""""

The initial histogram size for AWH biases depended (weakly) on the force
constant. This dependence has been removed, which increases the histogram
size by a about a factor of 3. In practice this has only a minor effect
on the time to solution. For multiple dimensions, the histogram size was
underestimated, in particular with a combination of slower and faster
dimensions. The, now simplified, formula for the initial histogram size is
given in the reference manual.

:issue:`3751`

Fixed LJ Ewald exclusions when used with cut-off electrostatics
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""

The exclusion forces in CUDA and OpenCL kernels were computed incorrectly
if LJ Ewald was used together with cut-off electrostatics.

:issue:`3840`
