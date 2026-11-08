local CBackend = {}

function CBackend:PutFormatted(State, Format, ...)
	CBackend.CCode = CBackend.CCode .. string.format(Format, ...)
end

function CBackend:Compile(State)
	CBackend.CCode = ""

	CBackend:PutFormatted(State, "Test\n")

	CBackend.Output = CBackend.CCode
	return CBackend.CCode
end

return CBackend
