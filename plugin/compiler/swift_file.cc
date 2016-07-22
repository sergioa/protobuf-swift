// Protocol Buffers for Swift
//
// Copyright 2014 Alexey Khohklov(AlexeyXo).
// Copyright 2008 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "swift_file.h"

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/stubs/strutil.h>

#include <sstream>

#include "swift_enum.h"
#include "swift_oneof.h"
#include "swift_extension.h"
#include "swift_helpers.h"
#include "swift_message.h"

namespace google { namespace protobuf { namespace compiler {namespace swift {
    
    FileGenerator::FileGenerator(const FileDescriptor* file)
    : file_(file), classname_(PackageFileName(file)) {
        
    }
    
    
    FileGenerator::~FileGenerator() {
        
    }
    
    void FileGenerator::GenerateSource(io::Printer* printer) {
        FileGenerator file_generator(file_);

        vector<string> tokens = FullNameSplit(file_);
        
        
        for (int i = 0; i < file_->message_type_count(); i++) {
            MessageGenerator(file_->message_type(i)).GenerateMessageIsEqualSource(printer);
        }
        
        //fields
        for (int i = 0; i < file_->extension_count(); i++) {
            ExtensionGenerator(ExtensionFileClassName(file_), file_->extension(i)).GenerateFieldsGetterSource(printer, FileClassName(file_));
        }
        
        for (int i = 0; i < file_->message_type_count(); i++) {
            MessageGenerator(file_->message_type(i)).GenerateGlobalStaticVariablesSource(printer, FileClassName(file_));
        }
        
        
        //Generate Messages with packages
        
        
        if (tokens.size() > 0) {
            printer->Print("$acontrol$ extension $package$ {\n",
                           "acontrol", GetAccessControlType(file_),
                           "package", PackageExtensionName(tokens));
            printer->Indent();
        }
        
        
        printer->Print("$acontrol$ struct $classname$ {\n",
                       "classname", classname_,
                       "acontrol", GetAccessControlType(file_));
        
        printer->Indent();
        printer->Print("$acontrol$ static var sharedInstance : $classname$ {\n"
                       " struct Static {\n"
                       "     static let instance : $classname$ = $classname$()\n"
                       " }\n"
                       " return Static.instance\n"
                       "}\n",
                       "classname", classname_,
                       "acontrol", GetAccessControlType(file_));
        
        for (int i = 0; i < file_->extension_count(); i++) {
            ExtensionGenerator(classname_, file_->extension(i)).GenerateFieldsSource(printer);
        }
        
        
        for (int i = 0; i < file_->message_type_count(); i++) {
            MessageGenerator(file_->message_type(i)).GenerateStaticVariablesSource(printer);
        }
        
        //TODO
        printer->Print("$acontrol$ var extensionRegistry:ExtensionRegistry\n",
                       "acontrol", GetAccessControlType(file_));
        printer->Print(
                       "\n"
                       "init() {\n");
        
        printer->Indent();
        
        
        for (int i = 0; i < file_->extension_count(); i++) {
            ExtensionGenerator(classname_, file_->extension(i)).GenerateInitializationSource(printer);
        }
        
        for (int i = 0; i < file_->message_type_count(); i++) {
            MessageGenerator(file_->message_type(i)).GenerateStaticVariablesInitialization(printer);
        }
        
        printer->Print("extensionRegistry = ExtensionRegistry()\n"
                       "registerAllExtensions(registry: extensionRegistry)\n");
        
        for (int i = 0; i < file_->dependency_count(); i++) {
            printer->Print("$dependency$.sharedInstance.registerAllExtensions(registry: extensionRegistry)\n",
                           "dependency", FileClassName(file_->dependency(i)));
        }
        
        printer->Outdent();
        printer->Print("}\n");
        
        
        printer->Print("$acontrol$ func registerAllExtensions(registry: ExtensionRegistry) {\n",
                       "acontrol", GetAccessControlType(file_));
        
        printer->Indent();
        for (int i = 0; i < file_->extension_count(); i++) {
            ExtensionGenerator(classname_, file_->extension(i)).GenerateRegistrationSource(printer);
        }
        
        for (int i = 0; i < file_->message_type_count(); i++) {
            MessageGenerator(file_->message_type(i)).GenerateExtensionRegistrationSource(printer);
        }
        printer->Outdent();
        printer->Print("}\n");
        
        for (int i = 0; i < file_->extension_count(); i++) {
            ExtensionGenerator(classname_, file_->extension(i)).GenerateMembersSourceExtensions(printer,classname_);
        }
        
        printer->Outdent();
        printer->Print("}\n\n");
        
        ///
        
        for (int i = 0; i < file_->enum_type_count(); i++) {
            EnumGenerator(file_->enum_type(i)).GenerateSource(printer);
        }
        
        
        for (int i = 0; i < file_->message_type_count(); i++) {
            MessageGenerator(file_->message_type(i)).GenerateSource(printer);
        }
        
       if (tokens.size() > 0) {
            printer->Outdent();
            printer->Print("}\n");
        }

        printer->Print(
                       "\n"
                       "// @@protoc_insertion_point(global_scope)\n");
    }
    
    
}  // namespace swift
}  // namespace compiler
}  // namespace protobuf
}  // namespace google