#pragma once

#include "AzCore/std/containers/array.h"
#include "AzCore/std/string/string_view.h"

static constexpr auto ExampleEventXmlBuffer = AZStd::string_view{ R"(
<SteamAudioDocument>
	<Event Name="hello_thar" Version="1">
		<Sound Name="sfx/those_chocodiles.raw" />
		<Sound Name="sfx/francine_those_chocodiles.raw" />
		<Task Type="StartTrigger" Value="oh_my_god_those_chocodiles" />
		<Task Type="StopTrigger" Value="francine_oh_my_god" />
		<Var Name="Volume" Value="1.0" />
	</Event>
</SteamAudioDocument>
      )" };
