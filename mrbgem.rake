MRuby::Gem::Specification.new('mruby-editline') do |spec|
  spec.license = 'MIT'
  spec.author  = 'Internet Initiative Japan Inc.'
  spec.linker.libraries << [ 'edit', 'termcap' ]
end                                   
