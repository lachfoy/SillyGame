#pragma once

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

template <typename T> struct JsonSerializer
{
	static void write(nlohmann::json &j, const char *name, const T &value)
	{
		j[name] = value;
	}

	static void read(const nlohmann::json &j, const char *name, T &value)
	{
		if (j.contains(name))
			j[name].get_to(value);
	}
};

template <> struct JsonSerializer<glm::vec3>
{
	static void write(nlohmann::json &j, const char *name, const glm::vec3 &v)
	{
		j[name] = {v.x, v.y, v.z};
	}

	static void read(const nlohmann::json &j, const char *name, glm::vec3 &v)
	{
		if (!j.contains(name) || !j[name].is_array() || j[name].size() != 3)
			return;

		v.x = j[name][0].get<float>();
		v.y = j[name][1].get<float>();
		v.z = j[name][2].get<float>();
	}
};

class SerializableParams
{
  public:
	SerializableParams() = default;
	SerializableParams(const SerializableParams &) = delete;
	SerializableParams &operator=(const SerializableParams &) = delete;
	SerializableParams(SerializableParams &&) = delete;
	SerializableParams &operator=(SerializableParams &&) = delete;

	void serialize(nlohmann::json &j) const
	{
		for (const auto &p : mProperties)
			p.write(j);
	}

	void deserialize(const nlohmann::json &j)
	{
		for (const auto &p : mProperties)
			p.read(j);
	}

  protected:
	template <typename T> void registerProp(const char *name, T &value)
	{
		PropertyRegistrar(
			this, name, [&value, name](nlohmann::json &j)
			{ JsonSerializer<T>::write(j, name, value); },
			[&value, name](const nlohmann::json &j)
			{ JsonSerializer<T>::read(j, name, value); });
	}

  private:
	struct Property
	{
		const char *name;
		std::function<void(nlohmann::json &)> write;
		std::function<void(const nlohmann::json &)> read;
	};

	std::vector<Property> mProperties;

	struct PropertyRegistrar
	{
		PropertyRegistrar(SerializableParams *owner, const char *name,
						  std::function<void(nlohmann::json &)> write,
						  std::function<void(const nlohmann::json &)> read)
		{
			owner->mProperties.emplace_back(Property{name, write, read});
		}
	};
};

#define REGISTER_PROP(field) registerProp(#field, field)
