-- Symbol names
local names_normal =
{
	en = "Inductor",
	cs = "Cívka"
}

local names_core =
{
	en = "Inductor with magnetic core",
	cs = "Cívka s magnetickým jádrem"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area = {-2, -1, 2, 0}

-- Terminals
local terminals = {{-2, 0}, {2, 0}}

-- Rendering
local render_normal = function (cr)
	-- The arcs
	cr.arc (-1.5, 0, 0.5, math.pi, 0)
	cr.arc (-0.5, 0, 0.5, math.pi, 0)
	cr.arc (0.5, 0, 0.5, math.pi, 0)
	cr.arc (1.5, 0, 0.5, math.pi, 0)
	cr.stroke ()
end

local render_core = function (cr)
	render_normal (cr)

	-- The core
	cr.move_to (-2, -1)
	cr.line_to (2, -1)
	cr.stroke ()
end

-- Register the symbols
logdiag.register ("Inductor",         names_normal, area, terminals, render_normal)
logdiag.register ("InductorWithCore", names_core,   area, terminals, render_core)


