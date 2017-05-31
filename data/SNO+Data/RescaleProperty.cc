void RescaleProperty(DBLinkPtr dbTable,
                     G4MaterialPropertiesTable* propertiesTable,
                     const std::string& property) {
  vector<double> scalings;
  try {
    scalings = dbTable->GetDArray(property + "_SCALING");
  }
  catch (DBNotFoundError& e) {
    warn << "No " + property + " for " + dbTable->GetIndex() << newline;
    return;
  }
  vector<double> reciprocalSum;
  vector<double> energies;
  for (size_t iScale=0; iScale<scalings.size(); iScale++) {
    stringstream propertyName;
    propertyName << property << iScale;
    G4MaterialPropertyVector* current = \
      propertiesTable->GetProperty(propertyName.str().c_str());
    if (current == NULL) // No guarantee this component has this property
      continue;
    if (reciprocalSum.empty()) {
      reciprocalSum.resize(current->GetVectorLength(), 0.0);
      energies.resize(current->GetVectorLength(), -1.0);
    }
    else if (reciprocalSum.size() != current->GetVectorLength())
      Log::Die("RescaleProperty: " + property + " components are different length arrays.");

    current->ScaleVector(1.0, 1.0 / scalings[iScale]); // Scale values (not energy)
    for (size_t bin=0; bin<current->GetVectorLength(); bin++) {
      reciprocalSum[bin] += 1.0 / (*current)[bin];
      energies[bin] = current->Energy(bin);
    }
  }

  // This is empty if no component data exists, in which case don't add the
  // property to the properties table
  if (reciprocalSum.empty())
    return;

  G4MaterialPropertyVector* total = new G4MaterialPropertyVector();
  for (size_t bin=0; bin<energies.size(); bin++)
    total->InsertValues(energies[bin], 1.0 / reciprocalSum[bin]);
  propertiesTable->AddProperty(property.c_str(), total);
}

