+++
date = "2018-12-17T10:56:57+01:00"
title = "Convert vtk data array to another vtk data array"
author = "Thomas Fischer"
+++

## General

Often, meshes contain geometrical data in common with data used for process
simulation. Usually, such data used by the process simulation is associated to
the mesh nodes or to the mesh cells. In the VTK unstructured grid file format
the geometrical and the process data information is stored in one file - in so
called data arrays.

Some tools, for instance ParaView, export data arrays always using a floating
point data type. OpenGeoSys expects the 'MaterialIDs' cell data array to have
int data-type.

Other tools, for instance GOCAD, export data associated with cells or nodes
sometimes as float. The tool can convert the cell data arrays to double
data-type.

## Usage

```bash
convertVtkDataArrayToVtkDataArray
    --in-mesh <file name>
    --existing-property-name <name of the cell data array as string>
    --out-mesh <filename for the output mesh>
    --new-property-name <name of the new cell data array>
    --new-property-data-type <data type as string>
```

## Example

```bash
convertVtkDataArrayToVtkDataArray
    -i doubleValuedMaterialIDs.vtu
    -e MaterialIDs_double
    --new-property-data-type int
    -n MaterialIDs
    -o intValuedMaterialIDs.vtu
```

<div class='note'>

### Example Files

[doubleValuedMaterialIDs.vtu](../../doubleValuedMaterialIDs.vtu)
</div>
