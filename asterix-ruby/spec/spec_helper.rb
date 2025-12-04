# frozen_string_literal: true

require 'simplecov'
SimpleCov.start do
  add_filter '/spec/'
  minimum_coverage 80
  minimum_coverage_by_file 70
end

require 'asterix'

RSpec.configure do |config|
  # Enable flags like --only-failures and --next-failure
  config.example_status_persistence_file_path = '.rspec_status'

  # Disable RSpec exposing methods globally on `Module` and `main`
  config.disable_monkey_patching!

  config.expect_with :rspec do |c|
    c.syntax = :expect
  end

  # Filter out backtrace from gems
  config.filter_run_when_matching :focus
  config.run_all_when_everything_filtered = true
end
